#!/usr/bin/env python3
"""Student-facing CI scope checks, public-score summary, and submission receipt."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess

TEMPLATE_REPOSITORY = 'unnc-aim/aim-2627-cpp-1-coursework'
FORM_TITLE = 'AIM 2627 CPP Coursework 1 回执'
FORM_URL = 'https://docs.qq.com/form/page/DWnRNY1RTZ2pvdmNK'
ALLOWED = re.compile(r'^(src/main/A1_(q[1-8]|b[12])\.cpp|README\.md|local_tests/.+|\.agent-sessions/bundles/.+)$')


def git(repo, *args):
    return subprocess.check_output(['git', '-C', str(repo), *args], text=True)


def baseline(repo):
    roots = git(repo, 'rev-list', '--max-parents=0', 'HEAD').splitlines()
    if not roots:
        raise ValueError('没有可用的初始 commit')
    return roots[-1]


def check_changes(repo, repository):
    if repository == TEMPLATE_REPOSITORY:
        print('模板维护仓库：跳过学生修改范围检查。')
        return 0
    base = baseline(repo)
    changed = git(repo, 'diff', '--name-only', '-z', base, 'HEAD').split('\0')
    bad = [path for path in changed if path and not ALLOWED.fullmatch(path)]
    print('对比基准（仓库初始 commit）：' + base)
    if bad:
        print('只允许修改 src/main 中原有题目的实现、README.md、local_tests/ 和 .agent-sessions/bundles/。')
        print('以下路径不在允许范围内：')
        for path in bad:
            print(repr(path))
        return 1
    print('修改范围检查通过。固定接口、实现区和文件内容限制仍以题面为准。')
    return 0


def commit_metrics(repo):
    base = baseline(repo)
    subjects = git(repo, 'log', base + '..HEAD', '--format=%s').splitlines()
    trivial = sum(bool(re.fullmatch(r'(update|fix|test|commit|修改|更新|调试)[^.!]{0,3}', subject))
                  for subject in subjects)
    return len(subjects), len(set(subjects)), trivial


def public_report(folder):
    reports = list(Path(folder).glob('*.json'))
    if len(reports) != 1:
        return None
    try:
        report = json.loads(reports[0].read_text())
    except (OSError, ValueError):
        return None
    if report.get('mode') != 'visible' or report.get('status') == 'ERROR':
        return None
    tasks = report.get('tasks', [])
    expected = {f'q{i}' for i in range(1,9)} | {'b1','b2'}
    if {task.get('id') for task in tasks} != expected or len(tasks) != 10:
        return None
    return report


def summary(repo, scores, grading_outcome='', scope_outcome=''):
    rows = ['# AIM 2627 C++ Coursework 1 · CI 回执', '',
            '## 公开用例自动评分', '',
            '**以下为公开自测分，不是正式成绩。** 正式功能分由教师私有 grader 复核，人工 40 分与最终总分待评。', '',
            '- 修改范围检查：' + (scope_outcome or '未提供 CI 状态'),
            '- 自动评分步骤：' + (grading_outcome or '未提供 CI 状态')]
    try:
        commit = git(repo, 'rev-parse', 'HEAD').strip()
        rows.append('- 本次检查 commit：`' + commit + '`')
    except (OSError, subprocess.SubprocessError):
        rows.append('- 本次检查 commit：无法读取')
    report = public_report(scores)
    if report:
        rows += ['', f'- 必做题原始分：**{report["raw_core"]}/100**',
                 f'- 公开自测功能分：**{report["automatic"]}/60**',
                 f'- 公开自测挑战分：**{report["bonus"]}/20**',
                 '- 人工评阅：**待评 /40**；最终总分：**待评**。', '',
                 '| 题目 | 类型 | 通过组数 | 原始得分 | 状态 |',
                 '| --- | --- | --- | --- | --- |']
        for task in report['tasks']:
            rows.append(f'| {task["id"].upper()} | '+('选做' if task['bonus'] else '必做')+
                        f' | {task["passed"]}/{task["total"]} | {task["score"]}/{task["max"]} | {task["status"]} |')
        rows += ['', '挑战题未完成不会阻止 CI 通过；必做题必须全部通过。逐组诊断见本次运行的 public-grading Artifact。']
    else:
        rows += ['', '**未生成完整公开评分报告，请查看自动评分步骤的日志。** 不能据此视为通过或零分。']
    rows += ['', '## 提交粒度（软检查）', '']
    try:
        count, unique, trivial = commit_metrics(repo)
        rows += [f'- 新增 commit 数：**{count}**（参考线 ≥15）',
                 f'- 去重后的 commit 信息：**{unique}**',
                 f'- 疑似过于简单的 commit 信息：**{trivial}**',
                 '- 此项只记录过程线索，不阻断 CI，也不自动决定人工分。']
    except (OSError, subprocess.SubprocessError, ValueError):
        rows.append('无法读取提交历史；请保留实际开发记录供教师复核。')
    rows += ['', '## 提交问卷', '',
             '完成必做题并确认 main 分支本次 CI 全部通过后，每人填写一次：',
             f'[{FORM_TITLE}]({FORM_URL})', '',
             '按问卷要求填写个人信息、仓库链接和最终 main commit，并保留对应 Actions 运行链接；以问卷实际字段为准。',
             'PR 检查供合并前自测，正式提交使用合并或推送后 main 分支的最终 commit。', '']
    return '\n'.join(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['check-files', 'summary'])
    parser.add_argument('--repo', type=Path, default=Path('.'))
    parser.add_argument('--scores', type=Path, default=Path('scores'))
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if args.command == 'check-files':
        return check_changes(args.repo, os.environ.get('GITHUB_REPOSITORY', ''))
    text = summary(args.repo, args.scores, os.environ.get('CI_GRADING_OUTCOME', ''),
                   os.environ.get('CI_SCOPE_OUTCOME', ''))
    if args.output:
        with args.output.open('a', encoding='utf-8') as stream:
            stream.write(text)
    else:
        print(text)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
