// NOTE: Assertions have been autogenerated by utils/update_mc_test_checks.py UTC_ARGS: --version 5
# RUN: not llvm-mc -triple=s390x %s 2>&1 | FileCheck %s

.machine
// CHECK: :[[@LINE-1]]:9: error: unexpected token in '.machine' directive

.machine pop
// CHECK: :[[@LINE-1]]:10: error: pop without corresponding push in '.machine' directive

.machine pop z13
// CHECK: :[[@LINE-1]]:14: error: expected newline

.machine 42
// CHECK: :[[@LINE-1]]:10: error: unexpected token in '.machine' directive

.machine z13+
// CHECK: :[[@LINE-1]]:13: error: expected newline
