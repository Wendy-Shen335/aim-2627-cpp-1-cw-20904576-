#!/usr/bin/env python3
"""只执行教师控制的编译与测试命令。不执行学生 CMake、脚本或 CI。"""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import re
import secrets
import shutil
import signal
import subprocess
import sys
import tempfile
import time

COMMON = '''#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
'''
MACROS = r'''
#define CHECK(...) do { if (!(__VA_ARGS__)) { std::cerr << "CHECK failed at " << __LINE__ << ": " << #__VA_ARGS__; return 1; } } while(false)
#define EXPECT_INVALID(...) do { bool got=false; try { (void)(__VA_ARGS__); } catch(const std::invalid_argument&) { got=true; } CHECK(got); } while(false)
#define EXPECT_RANGE(...) do { bool got=false; try { (void)(__VA_ARGS__); } catch(const std::out_of_range&) { got=true; } CHECK(got); } while(false)
'''

def limits(compile_mode):
    if os.name!='posix': return None
    def apply():
        import resource
        resource.setrlimit(resource.RLIMIT_CPU,(30,31) if compile_mode else (3,4))
        resource.setrlimit(resource.RLIMIT_FSIZE,(2*1024**2,2*1024**2) if not compile_mode else (64*1024**2,64*1024**2))
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
        if sys.platform.startswith('linux'):
            cap=(1536 if compile_mode else 512)*1024**2
            resource.setrlimit(resource.RLIMIT_AS,(cap,cap))
    return apply

def run_process(command,cwd,timeout,compile_mode=False):
    """磁盘限量日志、进程组超时清理；所有命令使用参数数组。"""
    with tempfile.TemporaryFile() as out:
        proc=subprocess.Popen(command,cwd=cwd,stdin=subprocess.DEVNULL,stdout=out,stderr=out,
                              start_new_session=os.name=='posix',preexec_fn=limits(compile_mode))
        try:
            code=proc.wait(timeout=timeout)
            status='PASS' if code==0 else 'FAIL'
        except subprocess.TimeoutExpired:
            if os.name=='posix': os.killpg(proc.pid,signal.SIGKILL)
            else: proc.kill()
            proc.wait(); code=-1; status='TIMEOUT'
        # 清理学生可能留下的同组后代。
        if os.name=='posix':
            try: os.killpg(proc.pid,signal.SIGKILL)
            except ProcessLookupError: pass
        out.seek(0); log=out.read(8000).decode('utf-8','replace')
        return {'status':status,'returncode':code,'log':log}

def grade(repo,manifest,teacher_root,compiler,only=None):
    repo=Path(repo).resolve()
    records=[]
    source_hashes={}
    source_files={}
    try:
        commit=subprocess.check_output(['git','-C',str(repo),'rev-parse','HEAD'],stderr=subprocess.DEVNULL,text=True,timeout=5).strip()
    except (subprocess.SubprocessError,OSError): commit=None
    with tempfile.TemporaryDirectory(prefix='aim-cpp-grade-') as tmp:
        tmp=Path(tmp)
        # CPP-Classroom exports are complete single-file sources. Stage only their implementation region.
        impl=tmp/'src'; impl.mkdir()
        invalid_sources={}
        for task in manifest['tasks']:
            tid=task['id']; filename=task.get('source')
            if filename:
                exported=re.compile(r'^(?:草稿|提交)-v[0-9]+-[0-9a-fA-F]{8}-'+re.escape(filename)+r'$')
                matches=[p for p in repo.rglob('*.cpp') if (p.name==filename or exported.fullmatch(p.name)) and p.is_file() and not p.is_symlink()]
                if len(matches)>1:
                    invalid_sources[tid]='Multiple files match '+filename+'; grade one student directory at a time.'
                    continue
                if not matches: continue
                source_files[tid]=str(matches[0].relative_to(repo))
                try:
                    original=matches[0].read_text(encoding='utf-8-sig')
                except (UnicodeError,OSError) as exc:
                    invalid_sources[tid]='Unable to read UTF-8 source: '+str(exc)
                    continue
                source_hashes[filename]=hashlib.sha256(matches[0].read_bytes()).hexdigest()
                begin='// BEGIN IMPLEMENTATION'; end='// END IMPLEMENTATION'
                if original.count(begin)!=1 or original.count(end)!=1 or original.index(begin)>=original.index(end):
                    invalid_sources[tid]='Implementation markers are missing, duplicated, or reversed.'
                    continue
                region=original.split(begin,1)[1].split(end,1)[0]
                staged='#pragma once\n#include "'+tid+'_types.hpp"\nnamespace aim::'+tid+' {\n'+region+'\n}\n'
                (impl/(tid+'.hpp')).write_text(staged,encoding='utf-8')
            else:
                source=repo/'src'/'main'/f"{tid}.hpp"
                if source.is_file() and not source.is_symlink():
                    shutil.copyfile(source,impl/source.name)
                    source_hashes[source.name]=hashlib.sha256(source.read_bytes()).hexdigest()
        fixed=teacher_root/'include'
        shutil.copytree(fixed,tmp/'include')
        for task in manifest['tasks']:
            tid=task['id']
            if only and tid not in only: continue
            result={'id':tid,'title':task['title'],'max':task['points'],'bonus':task['bonus'],'cases':[]}
            cases=task['cases']
            if tid in invalid_sources:
                result.update(status='INVALID_SOURCE',passed=0,total=len(cases),score=0,diagnostics=invalid_sources[tid])
                records.append(result); continue
            if not (impl/f'{tid}.hpp').is_file():
                result.update(status='MISSING',passed=0,total=len(cases),score=0)
                records.append(result); continue
            cpp=tmp/f'{tid}.cpp'; binary=tmp/(tid+'.exe')
            completion='__AIM_CASE_DONE_'+secrets.token_hex(12)+'__'
            blocks='\n'.join(f'case {i}: {{ {case["code"]} std::cout << "{completion}"; return 0; }}' for i,case in enumerate(cases))
            cpp.write_text(COMMON+f'\n#include "{tid}.hpp"\n'+MACROS+f'''
using namespace aim::{tid};
int main(int argc,char** argv) {{
    if(argc!=2) return 2;
    try {{ switch(std::stoi(argv[1])) {{
{blocks}
    default: return 2;
    }} }} catch(const std::exception& e) {{ std::cerr<<e.what(); return 3; }}
    catch(...) {{ std::cerr<<"unknown exception"; return 4; }}
}}
''',encoding='utf-8')
            compile_result=run_process([compiler,'-std=c++17','-O2','-Wall','-Wextra','-pedantic',
                '-I',str(tmp/'include'),'-I',str(impl),str(cpp),'-o',str(binary)],tmp,45,True)
            if compile_result['status']!='PASS':
                result.update(status='COMPILE_ERROR',passed=0,total=len(cases),score=0,diagnostics=compile_result['log'])
                records.append(result); continue
            for i,case in enumerate(cases):
                case_result=run_process([str(binary),str(i)],tmp,5)
                if case_result['status']=='PASS' and completion not in case_result['log']:
                    case_result['status']='EARLY_EXIT'
                    case_result['log']='测试未执行至完成点，提前退出不计通过。'+case_result['log']
                case_result['log']=case_result['log'].replace(completion,'')
                result['cases'].append({'name':case['name'],'visibility':case['visibility'],**case_result})
            passed=sum(c['status']=='PASS' for c in result['cases'])
            result.update(status='PASS' if passed==len(cases) else 'FAIL',passed=passed,total=len(cases),
                          score=round(task['points']*passed/len(cases),4))
            records.append(result)
    raw=sum(r['score'] for r in records if not r['bonus'])
    bonus=sum(r['score'] for r in records if r['bonus'])
    return {'schema_version':'aim-cpp-2627-classroom-v2','student':str(repo),'lesson':manifest['lesson'],
            'mode':manifest['mode'],'compiler':compiler,'submission_commit':commit,'source_sha256':source_hashes,'source_files':source_files,'tasks':records,
            'raw_core':round(raw,2),'raw_core_max':100,'automatic':round(raw*.6,2),'automatic_max':60,
            'bonus':round(bonus,2),'bonus_max':20,'ai_transparency':None,'quality_and_readme':None,
            'manual_max':40,'total':None,'note':'人工分未填写，总分不计算。可见测试分数仅供自测，正式评分使用私有 grader。'}

def grading_failed(reports, require_core=False):
    """CI may require core tasks while still reporting optional bonus failures."""
    return any(r.get('status')=='ERROR' or any(
        t['status']!='PASS' and (not require_core or not t['bonus'])
        for t in r.get('tasks',[])) for r in reports)


def main():
    parser=argparse.ArgumentParser(description='AIM C++17 独立批改器')
    parser.add_argument('repos',nargs='*',help='学生模板根目录，或 https://github.com/...git')
    parser.add_argument('--repos-file',type=Path)
    parser.add_argument('--output',type=Path,default=Path('scores'))
    parser.add_argument('--compiler',default=os.environ.get('CXX') or shutil.which('g++') or shutil.which('clang++'))
    parser.add_argument('--only',nargs='+',choices=[f'q{i}' for i in range(1,9)]+['b1','b2'])
    parser.add_argument('--require-core',action='store_true',help='仍评测挑战题，但仅必做题失败影响退出码，供 CI 使用')
    args=parser.parse_args()
    here=Path(__file__).resolve().parent
    manifest=json.loads((here/'cases.json').read_text())
    # grader/engine.py -> grader/template；template/tools/check.py -> template。
    teacher_root=here/'template' if (here/'template').is_dir() else here.parent
    repos=list(args.repos)
    if args.repos_file:
        repos += [s.strip() for s in args.repos_file.read_text().splitlines() if s.strip() and not s.lstrip().startswith('#')]
    repos=list(dict.fromkeys(repos))
    if not repos: parser.error('至少提供一个学生模板根目录')
    if not args.compiler or not shutil.which(args.compiler): parser.error('找不到 C++17 编译器，请用 --compiler 指定')
    args.output.mkdir(parents=True,exist_ok=True)
    reports=[]
    for source in repos:
        start=time.monotonic()
        name=re.sub(r'[^A-Za-z0-9_.-]+','_',source.rstrip('/').split('/')[-1]) or 'student'
        key=name+'-'+hashlib.sha256(source.encode()).hexdigest()[:8]
        try:
            if source.startswith('https://github.com/'):
                with tempfile.TemporaryDirectory(prefix='aim-cpp-clone-') as tmp:
                    clone=subprocess.run(['git','clone','--depth','1','--',source,tmp],capture_output=True,text=True,timeout=90)
                    if clone.returncode: raise RuntimeError(clone.stderr[:1000])
                    report=grade(tmp,manifest,teacher_root,args.compiler,args.only)
                    report['student']=source
            else:
                if not Path(source).is_dir(): raise ValueError('学生目录不存在')
                report=grade(source,manifest,teacher_root,args.compiler,args.only)
        except Exception as exc:
            report={'student':source,'mode':manifest['mode'],'status':'ERROR','error':str(exc),'automatic':0,'bonus':0}
        report['elapsed_seconds']=round(time.monotonic()-start,2)
        (args.output/f'{key}.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
        reports.append(report)
        print(f"{source}: auto={report['automatic']}/60 bonus={report['bonus']}/20 {report.get('status','')}",flush=True)
    rows=['# C++ Assessment 批改汇总','',f"模式：{manifest['mode']}。人工分为待录入，自动分不代表总分。",'',
          '| 提交 | 自动分 /60 | Bonus /20 | 状态 |','| --- | --- | --- | --- |']
    for r in reports: rows.append(f"| {r['student'].replace('|','/')} | {r['automatic']} | {r['bonus']} | {r.get('status','已批改')} |")
    (args.output/'summary.md').write_text('\n'.join(rows)+'\n')
    with (args.output/'manual_scores.csv').open('w',newline='') as f:
        writer=csv.writer(f);writer.writerow(['student','automatic_60','bonus_20','ai_transparency_20','quality_readme_20','total_120'])
        for r in reports: writer.writerow([r['student'],r['automatic'],r['bonus'],'','',''])
    return int(grading_failed(reports,args.require_core))

if __name__=='__main__': sys.exit(main())
