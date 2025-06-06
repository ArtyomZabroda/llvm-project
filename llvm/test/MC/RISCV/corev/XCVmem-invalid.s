# RUN: not llvm-mc -triple=riscv32 --mattr=+xcvmem %s 2>&1 \
# RUN:        | FileCheck %s --check-prefixes=CHECK-ERROR

cv.lb t0, (0), 0
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.lb 0, (t1), 0
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lb 0, (0), t2
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lb t0, (t1), -2049
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lb t0, (t1), 2048
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lb t0, (0), t1
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.lb 0, (t1), t1
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lb t0
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lb t0, (t2)
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lb t0, (t1), t2, t3
# CHECK-ERROR: :[[@LINE-1]]:21: error: invalid operand for instruction

cv.lbu t0, (0), 0
# CHECK-ERROR: :[[@LINE-1]]:12: error: invalid operand for instruction

cv.lbu 0, (t1), 0
# CHECK-ERROR: :[[@LINE-1]]:8: error: invalid operand for instruction

cv.lbu 0, (0), t0 
# CHECK-ERROR: :[[@LINE-1]]:8: error: invalid operand for instruction

cv.lbu t0, (t1), -2049
# CHECK-ERROR: :[[@LINE-1]]:18: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lbu t0, (t1), 2048
# CHECK-ERROR: :[[@LINE-1]]:18: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lbu t0, (0), t1
# CHECK-ERROR: :[[@LINE-1]]:12: error: invalid operand for instruction

cv.lbu 0, (t1), t1
# CHECK-ERROR: :[[@LINE-1]]:8: error: invalid operand for instruction

cv.lbu t0
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lbu t0, (t2)
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lbu t0, (t1), t2, t3 
# CHECK-ERROR: :[[@LINE-1]]:22: error: invalid operand for instruction

cv.lh t0, (0), 0
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.lh 0, (t1), 0
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lh 0, (0), t2
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lh t0, (t1), -2049
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lh t0, (t1), 2048
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lh t0, (0), t1
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.lh t0, t1(0)
# CHECK-ERROR: :[[@LINE-1]]:14: error: expected GPR register

cv.lh 0, (t1), t1
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lh t0
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lh t0, (t1)
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lh t0, (t1), t2, t3
# CHECK-ERROR: :[[@LINE-1]]:21: error: invalid operand for instruction

cv.lhu t0, (0), 0
# CHECK-ERROR: :[[@LINE-1]]:12: error: invalid operand for instruction

cv.lhu 0, (t1), 0
# CHECK-ERROR: :[[@LINE-1]]:8: error: invalid operand for instruction

cv.lhu 0, 0(t1)
# CHECK-ERROR: :[[@LINE-1]]:8: error: invalid operand for instruction

cv.lhu t0, (t1), -2049
# CHECK-ERROR: :[[@LINE-1]]:18: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lhu t0, (t1), 2048
# CHECK-ERROR: :[[@LINE-1]]:18: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lhu t0, (0), t1
# CHECK-ERROR: :[[@LINE-1]]:12: error: invalid operand for instruction

cv.lhu t0, t1(0)
# CHECK-ERROR: :[[@LINE-1]]:15: error: expected GPR register

cv.lhu 0, t0, t1
# CHECK-ERROR: :[[@LINE-1]]:13: error: expected '(' or invalid operand

cv.lhu t0
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lhu t0, (t1)
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lhu t0, (t1), t2, t3
# CHECK-ERROR: :[[@LINE-1]]:22: error: invalid operand for instruction

cv.lw t0, (0), 0
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.lw 0, (t1), 0
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lw 0, (0), t2
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lw t0, (t1), -2049
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lw t0, (t1), 2048
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.lw t0, (0), t1
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.lw t0, t1(0)
# CHECK-ERROR: :[[@LINE-1]]:14: error: expected GPR register

cv.lw 0, (t0), t1
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.lw t0
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lw t0, (t1)
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lw t0, (t1), t2, t3
# CHECK-ERROR: :[[@LINE-1]]:21: error: invalid operand for instruction

cv.sb t0, (0), 0
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sb 0, (t0), 0
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.sb t0, 0(t1)
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sb t0, (t1), 2048
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.sb t0, (0), t1
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sb 0, (t1), t1
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.sb t0
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.sh t0, (0), 0
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sh 0, (t1), 0
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.sh t0, 0(t1)
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sh t0, (t1), 2048
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.sh t0, (0), t1
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sh 0, (t1), t1
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.sh t0
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.sw t0, (0), 0
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sw 0, (t1), 0
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.sw t0, 0(t1)
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sw t0, (t1), 2048
# CHECK-ERROR: :[[@LINE-1]]:17: error: operand must be a symbol with %lo/%pcrel_lo/%tprel_lo specifier or an integer in the range [-2048, 2047]

cv.sw t0, (0), t1
# CHECK-ERROR: :[[@LINE-1]]:11: error: invalid operand for instruction

cv.sw 0, (t1), t1
# CHECK-ERROR: :[[@LINE-1]]:7: error: invalid operand for instruction

cv.sw t0
# CHECK-ERROR: :[[@LINE-1]]:1: error: too few operands for instruction

cv.lb t0, f0(t1)
# CHECK-ERROR: :[[@LINE-1]]:11: error: expected GPR register

cv.sb t0, t0(f1)
# CHECK-ERROR: :[[@LINE-1]]:14: error: expected GPR register
