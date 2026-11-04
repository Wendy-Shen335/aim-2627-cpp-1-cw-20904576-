# AIM 2627 C++ Assessment 01

Sentry Restart: A Night on the Data Link

## 0 How to read this statement

This is the formal specification of tasks, interfaces, scoring, and submission. See the student README for setup and commands. Copper-colored stories supply motivation; numbered clauses define acceptance. Examples illustrate typical behavior rather than replacing the contract. Private tests check only published behavior.

This assessment covers fundamental syntax and procedural data processing. Plain structs with data fields are allowed. Do not introduce custom classes, constructors, access-control design, inheritance, polymorphism, or ownership design. Use the supplied standard string/vector/array/stream patterns without studying their internal classes. Function wrappers are provided. B1 references and B2 array pointers are optional bridges.

Read background and task, then interfaces and clauses, and verify examples in order. Examples show direct function calls rather than stdin text. PUBLIC 2/2 means two embedded public groups passed, not a full formal score.

### Task overview

| ID | Task | Topics | Raw weight |
| --- | --- | --- | --- |
| Q1 | Normalize fixed-point telemetry | Integer arithmetic, branching, string assembly | 8 |
| Q2 | Maintain device labels | String search, replacement, append/delete, complete-field boundaries | 10 |
| Q3 | Detect anomalous windows | Sliding windows, indexing, time complexity | 10 |
| Q4 | Execute commands transactionally | Parsing, branching, transactional updates, counting | 12 |
| Q5 | Compute grid neighborhood risk | Matrices, nested loops, boundaries, exceptions | 12 |
| Q6 | Select a priority target | Filtering, lexicographic priorities, integer width | 14 |
| Q7 | Simulate heat and ammunition | Loop simulation, update order, resource accounting | 16 |
| Q8 | Build a telemetry processing pipeline | Range validation, deduplication, classification, id ordering, count invariants | 18 |
| B1 | Challenge: parse a signed decimal integer | Characters, output references, overflow prevention (optional) | +10 |
| B2 | Challenge: stable in-place compaction | Array pointers, in-place processing, stability (optional) | +10 |

### Execution, input, and output

Use C++17 and only its standard library, without hardware or external file access. Implementations communicate through arguments, return values, or object state; do not read stdin or print debugging output. Inputs outside stated domains are not assessed unless behavior is explicitly specified. Published invalid cases, exception types, endpoints, and failure states are contractual.

CPP-Classroom runs one complete .cpp per task, containing fixed types, the implementation region, checks, and main. Edit only between BEGIN IMPLEMENTATION and END IMPLEMENTATION; preserve markers, filenames, and interfaces. Ordinary helpers may be added inside; Assessment 2 may add necessary private members. The formal grader extracts this region and recompiles it with trusted types, independently of the student main.

Tasks do not depend on other answers, earlier tests, or shared mutable globals. Persistent state belongs to its object. Throw only when specified; Boolean failure must not silently become an exception.

## Prologue

In 2027, AIM is already counting down to its next competition trip. The previous replay freezes on an embarrassing scene: plentiful targets, rapidly scrolling logs, and a sentry that never moves. “Process running” clearly does not mean “robot working.”

Python training separated perception, navigation, decisions, and missions. C++ now takes over the data link closer to devices. You do not need to build an entire robot overnight. Instead, address a connected set of engineering problems: replayed events, untrusted readings, half-applied commands, and unexplained firing requests.

You are on data-link duty. Give each input a clear classification, each update an explicit rule, and the replay an auditable summary. Everything runs offline without hardware. Stories provide motivation; numbered clauses define acceptance.

## Q1 Normalize fixed-point telemetry · 8 raw points

### Background

On the night before departure for the 2027 league, the familiar startup sound finally returns. Someone celebrates; someone else points at 103% health and -7% battery. Different modules finish initialization at different times, and stale readings can arrive before configuration.

The Python sentry can already send notifications. You are moving its critical data path to C++, beginning with a predictable startup report. Every separator belongs to a protocol consumed by another program. A report that merely looks plausible in a terminal is insufficient.

### Your task

Implement the ordinary functions `ratio` and `report`. The first computes an integer health percentage; the second builds a startup record. The caller supplies arguments and reads return values. No interactive input or custom type is required.

**Topics:** Integer arithmetic, branching, string assembly. File: `src/main/A1_q1.cpp`.

### Fixed interface and data

```cpp
long long ratio(long long hp, long long max_hp);
std::string report(int id, long long hp, long long max_hp, int battery);
```

Use the existing `aim::q1` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. The magnitudes of hp and max_hp are at most 10^12. id is in [0,9999]; battery may be any int.

2. For max_hp<=0, ratio returns -1 without division. The HP field of report is INVALID, while battery processing still proceeds.

3. Otherwise clamp hp to [0,max_hp], then take the integer part of hp*100/max_hp. Do not use floating-point arithmetic or rounding.

4. Clamp battery to [0,100] first. Values >=60 give OK, values from 20 through 59 give WARNING, and lower values give LOW.

5. Return exactly ID=<id>;HP=<percentage or INVALID>;BAT=<normalized battery>;STATE=<category>, with no zero padding, spaces, or trailing newline.

6. Do not modify inputs or carry results between calls.

### Examples and explanation

```cpp
ratio(2, 3);                 // 66
report(7, 2, 3, 20);          // "ID=7;HP=66;BAT=20;STATE=WARNING"
report(0, 4, 0, -7);          // "ID=0;HP=INVALID;BAT=0;STATE=LOW"
```

Two thirds becomes 66 by truncation, and battery exactly 20 belongs to WARNING. With an invalid maximum, preserve the rest of the report: health is INVALID and negative battery becomes zero and LOW.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `ratio(-5,100)` | `0` | Clamp negative health first |
| `ratio(200,100)` | `100` | Saturate at full health |
| `ratio(10,0)` | `-1` | Invalid maximum; no division |
| `battery=59 / 60` | `WARNING / OK` | Classify after normalization |

### Self-check and notes

Check negative and excessive hp, zero maximum, and battery 19/20/59/60. Explain the integer width required by the intermediate multiplication.
END HIDDEN BOUNDARY HINTS -->

## Q2 Maintain device labels · 10 raw points

### Background

The terminal lists camera, inertial, and radar modules as cam,imu,radar. Hardware changes require renaming labels, removing retired modules, and appending new ones while keeping the existing status display.

An early script changes cam2 when asked to rename cam, and leaves a trailing comma after deleting the last device. Another caches positions before replacing a short name with a longer one, so later edits hit the wrong characters. Maintain the current text in order, match complete labels, and handle empty lists and duplicate labels predictably. The caller supplies edit records directly; no command parser is needed.

### Your task

Implement `edit_labels` to apply plain `LabelEdit` records to comma-separated labels in order and return the final string. from and to identify old and new labels; empty values select append or delete. Preserve the caller's original text and edit records.

**Topics:** String search, replacement, append/delete, complete-field boundaries. File: `src/main/A1_q2.cpp`.

### Fixed interface and data

```cpp
struct LabelEdit { std::string from; std::string to; };
std::string edit_labels(std::string labels, const std::vector<LabelEdit>& edits);
```

Use the existing `aim::q2` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. labels is empty or contains nonempty labels joined by single commas, initially at most 1000 characters. There are no leading, trailing, or repeated commas. Each case-sensitive label has 1..32 ASCII letters, digits, or underscores.

2. edits has at most 200 records. Each from/to is empty or satisfies the same label rules. Inputs are guaranteed to satisfy this grammar; format-error reporting, numeric conversion, and stream parsing are unnecessary.

3. Apply edits in order. Empty from with nonempty to appends to, inserting a comma only if the list is nonempty. Two empty values do nothing.

4. For nonempty from, find only the first complete label exactly equal to from in the current text. A partial substring is not a match. A missing label leaves text unchanged; even from equal to to processes only one occurrence once.

5. If found with nonempty to, replace that label. Empty to deletes the label and the necessary adjacent comma. Deleting the only label yields an empty string. Results have no leading, trailing, or repeated commas.

6. Duplicates are allowed; appending and renaming do not deduplicate. Preserve other labels' relative order. Each edit uses the result of the previous edit rather than cached character positions.

7. Empty edits returns the original text. Do not modify the caller's text or edits; calls are independent. You may use string operations such as find, substr, replace, erase, clear, and concatenation; no particular search algorithm is required.

### Examples and explanation

```cpp
edit_labels("cam,imu,radar", {{"imu","gyro"},{"cam",""},{"","lidar"}});
// "gyro,radar,lidar"
edit_labels("cam,cam2,cam", {{"cam","camera"},{"cam",""}});
// "camera,cam2"
edit_labels("cam", {{"cam",""},{"","imu"}}); // "imu"
```

The first sequence produces cam,gyro,radar, then gyro,radar, then gyro,radar,lidar. The second renames only the first cam to camera, then removes the remaining complete cam without touching cam2 or camera. The third deletes the sole label before appending imu, with no leading comma.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `edit_labels("cam2,cam",{{"cam","imu"}})` | `"cam2,imu"` | Match complete labels only |
| `edit_labels("a,b,a",{{"a","x"}})` | `"x,b,a"` | Edit only the first match |
| `edit_labels("a,b",{{"b",""}})` | `"a"` | Delete the adjacent comma at the end |
| `edit_labels("a",{{"a",""},{"","b"}})` | `"b"` | Append correctly after clearing |

### Self-check and notes

Cover deletion at every position, the only label, partial substrings, case, duplicates, missing labels, empty edits, and edits after a length-changing rename. Explain why position zero is a match and why later edits must search the updated text.
END HIDDEN BOUNDARY HINTS -->

## Q3 Detect anomalous windows · 10 raw points

### Background

A full-match average looks healthy, yet the sentry sometimes behaves as if power disappeared. Short bursts are hidden by the average. The electronics team asks for fixed-length rolling-window alarms so the relevant video frames can be inspected.

Negative readings represent deviation from a baseline and remain valid. Match logs are long; summing every window from scratch would make analysis too slow. Produce exact alarms with processing time linear in the number of frames.

### Your task

Implement `alarms` to return the zero-based right-end indexes of length-k windows whose sums exceed the threshold. Return frame indexes, not window numbers, left endpoints, or an alarm count.

**Topics:** Sliding windows, indexing, time complexity. File: `src/main/A1_q3.cpp`.

### Fixed interface and data

```cpp
std::vector<int> alarms(const std::vector<int>& values, int k, long long threshold);
```

Use the existing `aim::q3` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. n is at most 200000; each value is in [-10^9,10^9]. The threshold magnitude is at most 10^15 and k is an int.

2. For k<1 or k>n, including empty input, return an empty sequence without forming partial windows.

3. Windows are [0,k-1], [1,k], through [n-k,n-1]; overlap is allowed.

4. Alarm only when the sum is strictly greater than threshold. Equality does not alarm.

5. Return right-end indexes in ascending order, at most once per window, and leave input unchanged.

6. Require O(n) time and O(1) auxiliary working space, excluding the returned sequence. Use a sufficiently wide integer for sums.

### Examples and explanation

```cpp
alarms({1,2,3,4}, 2, 4);    // {2,3}
alarms({2,2}, 2, 4);        // {}
alarms({-4,-3,-2}, 2, -6);  // {2}
```

The first sums are 3,5,7 at indexes 1,2,3, so indexes 2 and 3 alarm. Equality produces no alarm. In the negative example, -5 exceeds -6; negative sums are not automatically safe.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `values={2,2}, k=2, threshold=4` | `{}` | Equality does not alarm |
| `values={-4,-3,-2}, k=2, threshold=-6` | `{2}` | Second window sums to -5 |
| `values={1}, k=0` | `{}` | Invalid window length |
| `values={1,2}, k=1, threshold=1` | `{1}` | Zero-based indexes |

### Self-check and notes

Cover k=1, k=n, k=0, all-negative input, and large sums. Explain why your implementation is not O(nk).
END HIDDEN BOUNDARY HINTS -->

## Q4 Execute commands transactionally · 12 raw points

### Background

A test script says: add five, set eight, reset, subtract two. The SET line also contains an accidental debug field. An old parser updates the value as soon as it reads SET 8 and discovers the invalid suffix too late.

Use an accumulator to practice transactional commands: validate syntax, arguments, and the proposed final range before committing any change. Rejected commands must also be counted so replay statistics account for every request.

### Your task

Implement `execute` and return a `CommandResult` containing the final value and accepted/rejected command counts. The struct carries data only and needs no member functions.

**Topics:** Parsing, branching, transactional updates, counting. File: `src/main/A1_q4.cpp`.

### Fixed interface and data

```cpp
struct CommandResult { long long value; int accepted; int rejected; };
CommandResult execute(const std::vector<std::string>& lines);
```

Use the existing `aim::q4` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. Start at zero and process at most 10000 lines. Each line is one command; an empty line is rejected.

2. Accept only case-sensitive ADD x, SET x, and RESET, allowing surrounding stream whitespace. ADD and SET require exactly one argument; RESET takes none.

3. Parse decimal x with standard stream extraction and require [-10^6,10^6]. Reject extra or missing fields, decimal suffixes, unknown operations, and overflow.

4. The candidate result is old+x for ADD, x for SET, and zero for RESET. It must also lie in [-10^6,10^6].

5. Commit the value and increment accepted only after complete validation. Otherwise increment rejected and preserve value.

6. Empty input returns {0,0,0}. accepted+rejected equals the number of lines, and calls are independent.

### String-stream guidance

`std::istringstream` treats an existing string as an input stream. Extraction works like `std::cin >>` without reading the keyboard. This example only demonstrates checking one integer and trailing content:

```cpp
std::istringstream input("12 extra");
int number = 0;
std::string extra;
bool has_number = static_cast<bool>(input >> number);
bool has_extra = static_cast<bool>(input >> extra);
// has_number=true, number=12, has_extra=true
```

The first success only means that a number was extracted. The second succeeds because extra content remains. Whitespace alone does not produce another word. For `12x`, integer extraction can yield 12 while a later extraction reads x, so the suffix must be checked. A missing number or an integer outside the extraction type's range makes numeric extraction fail. Create a separate stream for each command and validate its operation, arguments, suffix, and range before updating the accumulator.

### Examples and explanation

```cpp
execute({"ADD 5","SET 8 x","RESET","ADD -2"}); // {-2,3,1}
execute({"SET 1000000","ADD 1"});            // {1000000,1,1}
```

The first states are 5,5,0,-2: the malformed SET changes nothing. In the second example, argument 1 is valid but its proposed result is out of range, so the previous value remains 1000000.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `{"SET 5","RESET x"}` | `{5,1,1}` | Invalid RESET preserves value |
| `{"SET 1000000","ADD 1"}` | `{1000000,1,1}` | Valid argument, invalid result |
| `{"add 5",""}` | `{0,0,2}` | Case-sensitive; empty is rejected |
| `{"ADD +2","RESET"}` | `{0,2,0}` | A valid reset is accepted |

### Self-check and notes

Use a sequence to demonstrate continued processing after failure. Cover RESET x, add 5, ADD +2, and reset after an out-of-range command.
END HIDDEN BOUNDARY HINTS -->

## Q5 Compute grid neighborhood risk · 12 raw points

### Background

The sentry keeps driving close to pillars. Before building a full navigation cost map, the team wants a simple risk display: each traversable cell counts adjacent obstacles.

One implementation counts diagonals; another treats the edge as a wall. Both pictures look plausible but mean different things downstream. Follow the exact neighborhood and validation contract. Generate a risk map without finding paths or silently repairing input.

### Your task

Implement `risk` to turn a character map into an integer matrix of the same dimensions. Both input and output use [row][column] indexing.

**Topics:** Matrices, nested loops, boundaries, exceptions. File: `src/main/A1_q5.cpp`.

### Fixed interface and data

```cpp
std::vector<std::vector<int>> risk(const std::vector<std::string>& grid);
```

Use the existing `aim::q5` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. The map has at most 300 rows and 300 columns. An empty row sequence is valid and returns an empty matrix. A nonempty map must have at least one column and equal row lengths.

2. Only . for free space and # for obstacles are allowed. Zero columns, unequal rows, or any other character throw std::invalid_argument.

3. Validate the complete map before computing risk; invalid characters in later rows still matter.

4. Obstacle cells produce -1. Free cells count obstacles immediately above, below, left, and right, giving 0..4.

5. Diagonals are excluded and out-of-bounds positions are not obstacles. Edges add no automatic risk.

6. Leave input unchanged and never return a partial map. Require O(rows*columns) time for valid input.

### Examples and explanation

```cpp
risk({".#", ".."});       // {{1,-1},{0,1}}
risk({"."});               // {{0}}
risk({".#.","#.#",".#."}); // center cell: 4
risk({"..", "."});         // throws std::invalid_argument
```

Only the top-right cell is blocked in the first map. Its left and lower neighbors have risk one; bottom-left has zero. A single free cell has risk zero. Unequal rows must be rejected instead of padded or truncated.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `grid={}` | `{}` | An empty row sequence is valid |
| `grid={""}` | `invalid_argument` | Zero-width nonempty map is invalid |
| `grid={".","#"}` | `{{1},{-1}}` | Vertical neighbors still count |
| `grid={"#.",".."}` | `{{-1,1},{1,0}}` | Diagonal obstacle excluded |

### Self-check and notes

Cover empty maps, empty rows, single rows/columns, all obstacles, and an invalid final character.
END HIDDEN BOUNDARY HINTS -->

## Q6 Select a priority target · 14 raw points

### Background

The camera detects armor plates reliably, but the turret oscillates between opponents. Vision prefers the closest target; decision logic prefers low health. The team adopts a deterministic replay rule: filter invalid candidates, then compare health, distance, and identifier.

This is a training protocol rather than a complete competition tactic. The same set of candidates must choose the same identifier even if its input order changes. Ties cannot depend on luck.

### Your task

Implement `select_target` to return one Target id. The plain records contain coordinates relative to the robot origin. Range uses Euclidean distance, not Manhattan distance.

**Topics:** Filtering, lexicographic priorities, integer width. File: `src/main/A1_q6.cpp`.

### Fixed interface and data

```cpp
struct Target { int id; int x; int y; int hp; bool visible; };
int select_target(const std::vector<Target>& targets, int max_range);
```

Use the existing `aim::q6` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. At most 100000 targets have unique nonnegative ids, coordinate magnitudes <=10^6, hp in [-100,100], and a Boolean visible field.

2. Return -1 immediately for negative max_range; nonnegative ranges are at most 10^6. Empty input also returns -1.

3. Eligible targets are visible, have hp>0, and satisfy x*x+y*y<=max_range*max_range, including the boundary.

4. Choose lower hp first; compare squared distance only when hp ties, then lower id only when both tie.

5. Return -1 if none qualify; otherwise return the id rather than its array index. Input order is not a tie-breaker.

6. Require O(n) time without modifying or sorting input. Compute squares and sums with a sufficiently wide integer type.

### Examples and explanation

```cpp
select_target({{7,3,4,20,true},{2,0,5,20,true}}, 5); // 2
select_target({{7,3,4,1,true},{2,0,0,2,true}}, 5);   // 7
select_target({{1,0,0,0,true}}, 1);                 // -1
```

Both first-example targets have squared distance 25 and equal health, so id 2 wins. In the second, lower health outranks shorter distance. A zero-health target is ineligible even when located at the origin.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `{{9,0,0,3,true},{1,0,0,3,true}}, range=0` | `1` | Id breaks the tie |
| `{{1,3,4,1,true}}, range=5` | `1` | Range boundary is inclusive |
| `{{1,0,0,0,true}}, range=1` | `-1` | Zero health is ineligible |
| `targets={}, range=5` | `-1` | No candidates |

### Self-check and notes

Test ties at every comparison level, negative range, invisibility, exact range boundaries, and million-scale coordinates.
END HIDDEN BOUNDARY HINTS -->

## Q7 Simulate heat and ammunition · 16 raw points

### Background

“I pressed fire” describes a request, not a guaranteed shot. Ammunition may be empty or heat may be too high. Even the order within a frame matters: cooling before checking can allow a shot that checking first would reject.

Implement the exact discrete simulation specified here. These are coursework rules and must not be replaced by another competition heat model. Final counters and remaining resources must agree with one another.

### Your task

Implement `simulate` and return a FireResult: actual shots, rejected firing requests, and final ammunition and heat after every frame has been processed.

**Topics:** Loop simulation, update order, resource accounting. File: `src/main/A1_q7.cpp`.

### Fixed interface and data

```cpp
struct FireResult { int shots; int rejected; int ammo; int heat; };
FireResult simulate(int ammo, int heat, int cool, int shot_heat, const std::vector<int>& requests);
```

Use the existing `aim::q7` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. ammo, heat, cool, and shot_heat are in [0,100]. There are at most 10000 binary requests.

2. Every frame begins with heat=max(0,heat-cool).

3. A zero request only cools. It consumes no ammunition and changes neither shots nor rejected.

4. A one request fires only if ammo>0 and cooled heat+shot_heat<=100. Equality at 100 is allowed.

5. A shot consumes one round, adds shot_heat, and increments shots. Failure only increments rejected: no ammunition or shot heat is consumed.

6. Start shots and rejected at zero. Empty requests cause no cooling and preserve initial resources. Calls are independent.

### Examples and explanation

```cpp
simulate(2,90,10,20,{1,1,1}); // {2,1,0,100}
simulate(3,100,20,10,{0});    // {0,0,3,80}
simulate(1,100,0,0,{1});      // {1,0,0,100}
```

In the first example frame 1 cools 90 to 80 and fires to 100. Frame 2 cools to 90, but adding 20 would exceed 100, so it rejects. Frame 3 cools to 80 and fires to 100. The result is two shots, one rejection, zero ammunition, and heat 100.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `simulate(4,80,10,30,{})` | `{0,0,4,80}` | No frames means no cooling |
| `simulate(0,50,10,20,{1})` | `{0,1,0,40}` | Cool before rejecting empty ammo |
| `simulate(1,100,0,0,{1})` | `{1,0,0,100}` | Zero shot heat is valid |
| `simulate(1,100,0,1,{1})` | `{0,1,1,100}` | Rejection consumes no ammunition |

### Self-check and notes

Trace request, cooled heat, shot decision, and ammunition for each frame. Cover zero cooling, zero shot heat, empty ammunition, and empty requests.
END HIDDEN BOUNDARY HINTS -->

## Q8 Build a telemetry processing pipeline · 18 raw points

### Background

The final replay runs, but its first-valid, duplicate, and invalid counts do not add up to the input size. Separate modules have classified the same record as both invalid and duplicate.

The communication module already supplies integer id and health fields as plain records. There is no text to parse, but an int can still contain an out-of-range identifier or an unacceptable health value. Assign each record to one category and produce an ordered id list. Later valid packets never replace the first accepted value; invalid records reserve no id, and no state may leak from an earlier batch.

### Your task

Implement summarize to combine range validation, classification, deduplication, health normalization, and id ordering for structured telemetry. Telemetry and Summary are supplied plain structs. Summary contains mutually exclusive valid/duplicate/invalid counts, total normalized health from first valid records, and ascending ids.

**Topics:** Range validation, deduplication, classification, id ordering, count invariants. File: `src/main/A1_q8.cpp`.

### Fixed interface and data

```cpp
struct Telemetry { int id; int hp; };
struct Summary { int valid; int duplicate; int invalid; long long total_hp; std::vector<int> ids; };
Summary summarize(const std::vector<Telemetry>& frames);
```

Use the existing `aim::q8` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. frames contains at most 20000 Telemetry records, each already holding int fields id and hp. Either field may be any int. Read the fields directly; text parsing is unnecessary.

2. A record is legal only if id is in [0,9999] and hp in [-1000,1000]. A range violation counts as invalid and reserves no id. Validate id before indexing a marker table.

3. Only after full validation check duplicates. A previously accepted id increments duplicate and changes no other output.

4. A first valid record increments valid, clamps hp to [0,100] for the long long total_hp, and retains its id.

5. Return ids in ascending numeric order, once per id, rather than arrival order. Either sorting or scanning a marker table in id order is allowed.

6. Each record belongs to exactly one of first-valid, legal-duplicate, or invalid. Always satisfy valid+duplicate+invalid=frames.size() and valid=ids.size(). Empty input gives zero counts, zero total, and empty ids.

7. Leave frames unchanged, start fresh on every call, and do not depend on another task's implementation or source. This task compiles and scores independently.

### Examples and explanation

```cpp
Summary s = summarize({{2,200},{1,-5},{2,10},{-1,30}});
// s.valid=2, s.duplicate=1, s.invalid=1
// s.total_hp=100, s.ids={1,2}
Summary t = summarize({{1,-1001},{1,30},{1,1001}});
// t.valid=1, t.duplicate=0, t.invalid=2
// t.total_hp=30, t.ids={1}
```

In the first batch, hp 200 is legal and contributes 100 after clamping; hp -5 contributes zero. The later {2,10} is duplicate, and {-1,30} has an invalid id. In the second batch, -1001 reserves no id, 30 is the first valid value, and 1001 is invalid even though its id is already seen.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `{{1,-1001},{1,30}}` | `valid=1, invalid=1, total_hp=30` | Invalid record reserves no id |
| `{{1,20},{1,-1001}}` | `valid=1, duplicate=0, invalid=1` | Validate before deduplicating |
| `{{2,5},{1,9}}` | `ids={1,2}, total_hp=14` | Sort ids, not arrival order |
| `{}` | `valid=duplicate=invalid=total_hp=0; ids={}` | Empty batch starts fresh |

### Self-check and notes

Check both count identities, invalid ids, range endpoints, legal negative health, corrections after invalid records, invalid data for seen ids, unordered ids, empty batches, and consecutive calls. Distinguish input-range validation from clamping for the total.
END HIDDEN BOUNDARY HINTS -->

## B1 Challenge: parse a signed decimal integer · +10 points

### Background

A single numeric conversion can ruin a replay: a large positive value overflows and becomes a negative control value. The team asks for a signed parser that detects overflow before unsafe arithmetic.

Conversion libraries are useful in real programs, but this optional challenge excludes them so you handle signs, digits, asymmetric bounds, and failure behavior yourself. No required task depends on this extension.

### Your task

Implement parse_i32. The Boolean indicates success, and the caller-provided output variable may be changed only on success. The supplied reference parameter is the location where a successful result is returned.

**Topics:** Characters, output references, overflow prevention (optional). File: `src/main/A1_b1.cpp`.

### Fixed interface and data

```cpp
bool parse_i32(const std::string& text, int& output);
```

Use the existing `aim::b1` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. Text length is at most 1000. Valid syntax is one optional + or - followed by at least one ASCII digit 0..9.

2. Allow leading zeros, +0, and -0. Reject all whitespace, decimal points, repeated signs, non-ASCII digits, and trailing characters. Empty text and a sign alone are invalid.

3. The result must fit [-2147483648,2147483647], inclusive; the negative and positive limits are asymmetric.

4. On success assign output and return true. On any failure return false and preserve its original value.

5. Do not use stoi/stol/stoll, string streams, or C numeric-conversion functions. Scan characters and reject before overflowing arithmetic.

6. Require O(text length) time, O(1) auxiliary space, and no custom class.

### Examples and explanation

```cpp
int value = 9;
parse_i32("-2147483648", value); // true; value=-2147483648
parse_i32("2147483648", value);  // false; value remains -2147483648
parse_i32("-0000", value);       // true; value=0
```

The minimum signed 32-bit integer is valid. One above the maximum positive value fails while preserving the previous result. Leading zeros are accepted, and negative zero becomes zero.

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `"2147483647"` | `true; output=2147483647` | Maximum is inclusive |
| `"-2147483648"` | `true; output=-2147483648` | Minimum is inclusive |
| `"2147483648"` | `false; output unchanged` | Prevent overflow before arithmetic |
| `"+" / " 1" / "1.0"` | `false; output unchanged` | Syntax is also contractual |

### Self-check and notes

Test both inclusive limits, one step outside each, a 1000-digit input, and sign-only strings.
END HIDDEN BOUNDARY HINTS -->

## B2 Challenge: stable in-place compaction · +10 points

### Background

A sensor buffer contains outliers. The communication code wants valid readings compacted at its front, with a new usable length. Sorting would destroy time order, and making a second buffer would consume extra memory.

You receive an existing integer array and its length. Rearrange it in place without replacing or freeing it. Retained values keep their sampling order. This optional pointer-and-array exercise introduces no ownership or object-oriented design.

### Your task

Implement compact to retain values in [low,high], move them stably to the front, and return their count. Only the prefix before the returned length belongs to the result.

**Topics:** Array pointers, in-place processing, stability (optional). File: `src/main/A1_b2.cpp`.

### Fixed interface and data

```cpp
std::size_t compact(int* data, std::size_t n, int low, int high);
```

Use the existing `aim::b2` namespace in the skeleton; do not nest it again. Fixed structs or base types shown above are already provided outside the implementation region.

### Contract

1. n<=200000. For n=0, data may be nullptr and must not be dereferenced. Otherwise it points to n valid int elements.

2. Retain low<=data[i]<=high inclusively, including duplicates.

3. Stability preserves relative order among retained elements; do not sort.

4. After returning k, data[0..k-1] is the result; the remaining tail is unspecified.

5. For low>high, return zero without modifying any element. No matches also returns zero.

6. Require O(n) time and O(1) extra space. No auxiliary container or new array, and do not free the caller's array.

### Examples and explanation

```cpp
int data[] = {-1,2,0,9};
std::size_t k = compact(data,4,0,2); // k=2; prefix {2,0}
compact(nullptr,0,0,1);             // 0
```

The retained values are 2 then 0, not sorted as 0 then 2. The result length is two and the tail is unspecified. Empty input returns zero without reading data[0].

<!-- BEGIN HIDDEN BOUNDARY HINTS
### Boundary behavior reference

| Input or situation | Required result | Reason |
| --- | --- | --- |
| `data={3,1,3,2,3}, range=[2,3]` | `k=4; prefix={3,3,2,3}` | Preserve duplicates and order |
| `data={1,2}, low=5, high=3` | `k=0; data={1,2}` | Reversed bounds leave all data unchanged |
| `data={1,1,1}, range=[1,1]` | `k=3` | Equal inclusive bounds are valid |
| `data=nullptr, n=0` | `k=0` | Do not dereference nullptr |

### Self-check and notes

Cover retaining all/none, duplicates, equal bounds, and reversed bounds. Explain why writing does not destroy unread data.
END HIDDEN BOUNDARY HINTS -->

## Submission and acceptance

### Development and submission

Keep task filenames under src/main. A successful CPP-Classroom run still requires active submission. Repository users submit the URL and final main commit; platform users submit their record and requested source export. The instructor announces the deadline.

Edit only implementation regions and the personal notes at the end of README; also commit automatically generated .agent-sessions/bundles/ archives. Put experimental tests in a new local_tests directory. Preserve public tests, fixed interfaces, markers, tools, and workflows. Keep reviewable incremental records by task or fix. Before submission, place your implementations in a fresh template and rerun checks.

Document each task's approach, boundaries, and complexity, at least three additional boundary tests, and one actual defect, diagnosis, and fix. AI assistance is allowed: disclose its scope, how you checked its output, and reviewable process records. If unused, explicitly declare this and supply development records. You must be able to explain your interfaces and failure paths.

### GitHub CI and submission receipt

Develop and push on main, or merge a pull request into main. CI runs on main pushes, pull requests targeting main, and manual dispatches. Following the Python coursework, it checks cumulative changed paths, runs automatic public grading, and writes a soft commit-granularity check to the Actions Summary.

Student repositories may change only existing task implementations under src/main, README.md, local_tests/, and .agent-sessions/bundles/. Preserve workflows, tools, public tests, fixed types, and statements. The instructor template repository skips the student path check. The initial commit is the history baseline; 15 later commits is a reference for review only and neither blocks CI nor automatically determines manual credit.

The Summary shows per-task passed groups, public automatic credit /60, and bonus credit /20, with JSON, Markdown, and CSV reports in an Artifact. All Q1–Q8 must pass. B1/B2 are optional and unfinished challenges do not block CI. Summaries and available diagnostics are retained even when grading fails.

Public self-check scores are not final grades. Private instructor tests never enter student repositories or CI; manual credit /40 and the final total remain pending. Once the final main commit passes CI, submit [AIM 2627 CPP Coursework 1 回执](https://docs.qq.com/form/page/DWnRNY1RTZ2pvdmNK) once per student. Supply personal information, repository URL, and final main commit as requested by the actual form, and retain the corresponding Actions run URL. A pull-request check does not replace the final main submission.

### AI assistance and Agent session records

AI assistance is allowed and encouraged when used responsibly. Its use alone does not reduce functional credit: you must verify the code and explain interfaces, state changes, and failure paths. The existing 20-point transparency block reviews disclosure, archive coverage, authenticity, and verification. If no AI was used, explicitly state this and retain development records; do not fabricate session archives.

As in the Python Assessment, use Agent Session Commit (AgentLedger), pinned to 0.1.5. Local archiving needs Git and Python 3.10+; running the C++ programs still needs only C++17. Configure each personal student repository, not the private instructor directory or an unrelated project. CPP-Classroom users also synchronize the actual submitted source into their personal Git repository and submit its incremental history and records.

Install in a persistent Python environment, then run the installer from the student repository root:

```bash
python -m pip install --index-url https://pypi.org/simple 'agent-session-commit[pre-commit]==0.1.5'
agent-session-commit install --pre-commit
agent-session-commit status
```

Select the Agent actually used and confirm its session source. Settings are local to each clone: configure each Assessment and new clone separately, and rerun the installer after upgrading. The supplied .pre-commit-config.yaml pins the archive backend to v0.1.5. Keep the Python environment available; moving or removing it requires reinstalling hooks.

After a successful normal git commit, the post-commit hook archives new session content under .agent-sessions/bundles/ and amends the same commit. Unchanged sessions produce no new bundle or extra archive commit. Amend changes the SHA, so use git rev-parse HEAD for the final revision. A healthy status alone does not establish complete recording: after a real change and commit, inspect the final commit for its corresponding archive.

```bash
git rev-parse HEAD
git show --stat HEAD -- .agent-sessions/
```

Rerun setup when changing Agent or source. Browser chats, unsupported IDEs, and remote environments are not automatically guaranteed coverage: export actual conversations in a supported format and select Custom when needed. Raw exports may use the ignored .agent-sessions/source/ directory; bundles must be committed. Do not ignore the whole .agent-sessions/ directory. In README, disclose tools, assistance scope, archive location, how results were checked, and any missing records with an explanation and supporting material.

Archives are ordinary, unencrypted Git content without automatic redaction; public pushes make them readable. Select this coursework's source only and exclude credentials and unrelated private sessions. Declare no AI use honestly, explain incomplete records, and do not invent or substitute conversations.

The course cross-checks archives, repository markers, and Git history. Transparent AI use is not penalized. A marker alone never replaces review of development records or determines functional or final grades.


### Scoring

Q1–Q8 have raw weights 8,10,10,12,12,14,16,18, totaling 100 and scaled by 0.6 to 60 automatic points. Each fixed test group earns partial credit; a seeded group passes only when all its assertions pass. B1 and B2 add ten each without scaling, for at most twenty bonus points.

Manual assessment contributes 40: process transparency 20 (complete disclosure 5, reviewable development/assistance records 10, verification explanation 5); code quality and explanation 20 (naming and organization 5, implementation/complexity compliance 5, boundary tests and defect analysis 5, design explanation 5). Students not using AI are assessed by equivalent development-record quality. Core maximum is 100 and total maximum including challenges is 120.

Passing public tests does not guarantee private success. Missing files, TODOs, incompatible interfaces, compilation errors, unexpected exceptions, timeouts, fabricated output, and early exits before test completion do not pass. Failure in one task does not block others. Manual fields remain blank and total remains null until instructor assessment; automatic points alone are not the final grade.

### Resources and review

The formal grader allows 45 wall-clock seconds per compilation and five per test group, with a 512 MiB execution address-space limit on Linux. Complexity and implementation restrictions also receive source review; a single fast run does not prove compliance. Classroom limits can differ; formal results use the instructor's consistent environment.

Report specification conflicts with a minimal reproduction, actual result, and relevant clause. Preserve tests and do not guess unpublished requirements.
