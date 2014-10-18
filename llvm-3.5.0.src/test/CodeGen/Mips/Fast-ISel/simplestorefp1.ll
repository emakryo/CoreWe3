; RUN: llc -march=mipsel -relocation-model=pic -O0 -mips-fast-isel -fast-isel-abort -mcpu=mips32r2 \
; RUN:     < %s | FileCheck %s

@f = common global float 0.000000e+00, align 4
@de = common global double 0.000000e+00, align 8

; Function Attrs: nounwind
define void @f1() #0 {
entry:
  store float 0x3FFA76C8C0000000, float* @f, align 4
  ret void
; CHECK:  .ent  f1
; CHECK:  lui  $[[REG1:[0-9]+]], 16339
; CHECK:  ori  $[[REG2:[0-9]+]], $[[REG1]], 46662
; CHECK:  mtc1  $[[REG2]], $f[[REG3:[0-9]+]]
; CHECK:  lw  $[[REG4:[0-9]+]], %got(f)(${{[0-9]+}})
; CHECK:  swc1  $f[[REG3]], 0($[[REG4]])
; CHECK:   .end  f1

}

; Function Attrs: nounwind
define void @d1() #0 {
entry:
  store double 1.234567e+00, double* @de, align 8
; CHECK:  .ent  d1
; CHECK:  lui  $[[REG1a:[0-9]+]], 16371
; CHECK:  ori  $[[REG2a:[0-9]+]], $[[REG1a]], 49353
; CHECK:  lui  $[[REG1b:[0-9]+]], 21403
; CHECK:  ori  $[[REG2b:[0-9]+]], $[[REG1b]], 34951
; CHECK:  mtc1  $[[REG2b]], $f[[REG3:[0-9]+]]
; CHECK:  mthc1  $[[REG2a]], $f[[REG3]]
; CHECK:  sdc1  $f[[REG3]], 0(${{[0-9]+}})
; CHECK:  .end  d1
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
