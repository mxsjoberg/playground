import json

def replay(old_state, latest_state, ops):
    if old_state == latest_state: return latest_state

    pos = 0
    for op in ops:
        if "count" in op and (pos + op["count"]) > len(old_state): return False
        # skip
        if op["op"] == "skip": pos = op["count"]
        # delete
        elif op["op"] == "delete": old_state = old_state[:pos] + old_state[pos + op["count"]:]
        # insert
        elif op["op"] == "insert": old_state = old_state[:pos] + op["chars"] + old_state[pos:]
    return old_state

tests = [
    [
        'Repl.it uses operational transformations to keep everyone in a multiplayer repl in sync.',
        'Repl.it uses operational transformations.',
        '[{"op": "skip", "count": 40}, {"op": "delete", "count": 47}]',
        'valid'
    ],
    [
        'Repl.it uses operational transformations to keep everyone in a multiplayer repl in sync.',
        'Repl.it uses operational transformations.',
        '[{"op": "skip", "count": 45}, {"op": "delete", "count": 47}]',
        'invalid'
    ],
    [
        'Repl.it uses operational transformations to keep everyone in a multiplayer repl in sync.',
        'Repl.it uses operational transformations.',
        '[{"op": "skip", "count": 40}, {"op": "delete", "count": 47}, {"op": "skip", "count": 2}]',
        'invalid'
    ],
    [
        'Repl.it uses operational transformations to keep everyone in a multiplayer repl in sync.',
        'We use operational transformations to keep everyone in a multiplayer repl in sync.',
        '[{"op": "delete", "count": 7}, {"op": "insert", "chars": "We"}, {"op": "skip", "count": 4}, {"op": "delete", "count": 1}]',
        'valid'
    ],
    [
        'Repl.it uses operational transformations to keep everyone in a multiplayer repl in sync.',
        'We can use operational transformations to keep everyone in a multiplayer repl in sync.',
        '[{"op": "delete", "count": 7}, {"op": "insert", "chars": "We"}, {"op": "skip", "count": 4}, {"op": "delete", "count": 1}]',
        'invalid'
    ],
    [
        'Repl.it uses operational transformations to keep everyone in a multiplayer repl in sync.',
        'Repl.it uses operational transformations to keep everyone in a multiplayer repl in sync.',
        '[]',
        'valid'
    ]
]

for test in tests:
    old_state = test[0]
    latest_state = test[1]
    ops = json.loads(test[2])
    expected_result = test[3]
    
    replay_state = replay(old_state, latest_state, ops)
    assert expected_result == 'valid' if latest_state == replay_state else 'invalid'
