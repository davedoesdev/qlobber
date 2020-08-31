/*globals global */

global.rabbitmq_test_bindings = [
    ["a.b.c",         "t1"],
    ["a.b.b.c",       "t4"],
    ["b.b.c",         "t13"],
    ["a.b.b",         "t14"],
    ["a.b",           "t15"],
    ["b.c",           "t16"],
    ["",              "t17"],
    ["vodka.martini", "t19"],
    ["a.b.c",         "t20"]
];

global.rabbitmq_expected_results_before_remove = [
    ["a.b.c",               ["t1",
                             "t20",
                             ]],
    ["a.b",                 ["t15",
                             ]],
    ["a.b.b",               ["t14",
                             ]],
    ["",                    ["t17"]],
    ["b.c.c",               []],
    ["a.a.a.a.a",           []],
    ["vodka.gin",           []],
    ["vodka.martini",       ["t19"]],
    ["b.b.c",               ["t13",
                             ]],
    ["nothing.here.at.all", []],
    ["oneword",             []]
];

global.rabbitmq_bindings_to_remove = [1, 5, 11, 19, 21];

global.rabbitmq_expected_results_after_remove = [
    ["a.b.c",               ["t20"
                             ]],
    ["a.b",                 ["t15"
                             ]],
    ["a.b.b",               ["t14"
                             ]],
    ["",                    ["t17"]],
    ["b.c.c",               []],
    ["a.a.a.a.a",           []],
    ["vodka.gin",           []],
    ["vodka.martini",       []],
    ["b.b.c",               ["t13"
                             ]],
    ["nothing.here.at.all", []],
    ["oneword",             []]
];

global.rabbitmq_expected_results_after_remove_all = [
    ["a.b.c",               []],
    ["a.b",                 ["t15"
                             ]],
    ["a.b.b",               ["t14"
                             ]],
    ["",                    ["t17"]],
    ["b.c.c",               []],
    ["a.a.a.a.a",           []],
    ["vodka.gin",           []],
    ["vodka.martini",       []],
    ["b.b.c",               ["t13"]],
    ["nothing.here.at.all", []],
    ["oneword",             []]
];

global.rabbitmq_expected_results_after_clear = [
    ["a.b.c", []],
    ["b.b.c", []],
    ["", []]
];

exports.test_bindings = global.rabbitmq_test_bindings;
