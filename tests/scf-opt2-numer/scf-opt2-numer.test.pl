#!/usr/bin/perl  

while ($ARGV = shift) {
  if   ("$ARGV" eq "-q") { $QUIET = 1; }
  elsif("$ARGV" eq "-i") { $SRC_PATH = shift; }
  elsif("$ARGV" eq "-x") { $EXEC_PATH = shift; }
}

if($SRC_PATH ne "") {
  require($SRC_PATH . "/../psitest.pl");
}
else {
  require("../psitest.pl");
}

# build the command for the psi3 driver
$PSICMD = build_psi_cmd($QUIET, $SRC_PATH, $EXEC_PATH);

$TOL = 10**-8;
$GTOL = 10**-8;
if($SRC_PATH ne "") {
  $REF_FILE = "$SRC_PATH/file11.ref";
}
else {
  $REF_FILE = "file11.ref";
}
$TEST_FILE = "psi.file11.dat";
$RESULT = "scf-opt2-numer.test";

system ("$PSICMD");

$FAIL = 0;

$natom = seek_natom_file11($REF_FILE,"iteration");

open(RE, ">$RESULT") || die "cannot open $RESULT $!"; 
select (RE);
printf "SCF-OPT-NUMER:\n";


if(abs(seek_energy_file11($REF_FILE,"iteration") - seek_energy_file11($TEST_FILE,"iteration")) > $TOL) {
  fail_test("SCF energy"); $FAIL = 1;
}
else {
  pass_test("SCF energy");
}

@geom_ref = seek_geom_file11($REF_FILE, "iteration");
@geom_test = seek_geom_file11($TEST_FILE, "iteration");
if(!compare_arrays(\@geom_ref, \@geom_test, $natom, 3, $GTOL)) {
  fail_test("SCF Geometry"); $FAIL = 1;
}
else {
  pass_test("SCF Geometry");
}

@grad_ref = seek_grad_file11($REF_FILE, "iteration");
@grad_test = seek_grad_file11($TEST_FILE, "iteration");

if(!compare_arrays(\@grad_ref, \@grad_test, $natom, 3, $GTOL)) {
  fail_test("SCF Gradient"); $FAIL = 1;
}
else {
  pass_test("SCF Gradient");
}
close (RE);

system("cat $RESULT");

exit($FAIL);
