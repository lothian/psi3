#!/usr/bin/perl 

# Some general functions useful for parsing PSI3 output files.
# TDC, 5/03

#
# Rules of use:
# 1) environment must define variable $SRCDIR which points to the source directory where
#    the input and reference output files are located
# 2) variable $EXECDIR can be used to specify location of Psi3 executables relative to the directory
#    where testing is performed
#

#
# Global definitions
#
use Env;
$SRC_PATH = $SRCDIR;
if ($SRC_PATH eq "") {
  $SRC_PATH = ".";
}
if ($EXECDIR eq "") {
  $PSITEST_EXEC_PATH = "../../bin";
}
else {
  $PSITEST_EXEC_PATH = $EXECDIR;
}
$PSITEST_DEFAULT_INPUT = "input.dat";
$PSITEST_DEFAULT_PREFIX = "psi";
if ($PSITEST_INPUT eq "") {
  $PSITEST_INPUT = $PSITEST_DEFAULT_INPUT;
}
if ($PSITEST_PREFIX eq "") {
  $PSITEST_PREFIX = $PSITEST_DEFAULT_PREFIX;
}
$PSITEST_TARGET_SUFFIX = "test";
$PSITEST_TEST_SCRIPT = "runtest.pl";

# These are definitions that default tester knows about -- should match Psi driver!
@PSITEST_JOBTYPES = ("SP", "OPT", "DISP", "FREQ", "SYMM_FC", "FC", 
"OEPROP", "DBOC");
@PSITEST_WFNS = ("SCF", "MP2", "MP2R12", "DETCI", "DETCAS", "CASSCF",
"RASSCF", "BCCD", "BCCD_T", "CC2", "CCSD", "CCSD_T", "CC3", "EOM_CC2", 
"LEOM_CC2", "EOM_CCSD", "LEOM_CCSD", "OOCCD", "CIS", "EOM_CC3");
@PSITEST_REFTYPES = ("RHF", "ROHF", "UHF", "TWOCON");
@PSITEST_DERTYPES = ("NONE", "FIRST", "SECOND", "RESPONSE");

$PSITEST_DEFAULT_NSTAB = 5;       # number of eigenvalues STABLE prints out

$PSITEST_ETOL = 10**-8;           # Default test criterion for energies
$PSITEST_ENUCTOL = 10**-13;       # Check nuclear repulsion energy tighter than other energies
$PSITEST_EEOMTOL = 10**-5;        # Less stringent test for EOM-CC energies
$PSITEST_GEOMTOL = 10**-6;        # Default test criterion for Cartesian geometries
$PSITEST_GTOL = 10**-6;           # Default test criterion for gradients
$PSITEST_HTOL = 10**-2;           # Default test criterion for Hessians
$PSITEST_POLARTOL = 10**-5;       # Default test criterion for polarizabilities
$PSITEST_OPTROTTOL = 10**-3;      # Default test criterion for optical rotation
$PSITEST_STABTOL = 10**-4;        # Default test criterion for Hessian eigenvalues
$PSITEST_MPOPTOL = 10**-5;        # Default test criterion for Mulliken populations
$PSITEST_CIDIPTOL = 10**-4;       # Default test criterion for CI dipoles
##################################################
#
# This is a "smart" tester -- it parses the input and figures out
# what kinds of tests to run
#
##################################################################
sub do_tests
{
  test_started();
  my $interrupted;
  ($interrupted) = run_psi_command(@_);

  # Figure out what calculation has been run -- run "psi3 -c" and get the calculation type string
  my $calctype;
  my $wfn;
  my $direct;
  my $ref;
  my $dertype;
  my $jobtype;
  ($calctype, $wfn, $ref, $jobtype, $dertype, $direct) = get_calctype_string();

  my $item;
  my $ok = 0;
  foreach $item (@PSITEST_JOBTYPES) {
    if ($item eq $jobtype) {$ok = 1;}
  }
  if ($ok != 1) {
    fail_test("Default Psi tester do_tests does not recognize jobtype $jobtype");
    test_finished(1,$interrupted);
  }

  $ok = 0;
  foreach $item (@PSITEST_DERTYPES) {
    if ($item eq $dertype) {$ok = 1;}
  }
  if ($ok != 1) {
    fail_test("Default Psi tester do_tests does not recognize dertype $dertype");
    test_finished(1,$interrupted);
  }

  $ok = 0;
  foreach $item (@PSITEST_WFNS) {
    if ($item eq $wfn) {$ok = 1;}
  }
  if ($ok != 1) {
    fail_test("Default Psi tester do_tests does not recognize wfn $wfn");
    test_finished(1,$interrupted);
  }

  $ok = 0;
  foreach $item (@PSITEST_REFTYPES) {
    if ($item eq $ref) {$ok = 1;}
  }
  if ($ok != 1) {
    fail_test("Default Psi tester do_tests does not recognize reference $ref");
    test_finished(1,$interrupted);
  }


  my $fail = 0;
    
  SWITCH1: {

    if ($jobtype eq "OPT") {
      
      if ($dertype eq "NONE" || $dertype eq "FIRST") {
        $fail |= compare_energy_file11($wfn);
        $fail |= compare_geom_file11($wfn);
        $fail |= compare_grad_file11($wfn);
      }

      last SWITCH1;
    }

    # I'll lump all single-point computations together
    if ( $jobtype eq "SP" || $jobtype eq "FREQ" || $jobtype eq "SYMM_FC" || $jobtype eq "OEPROP" ||
         $jobtype eq "DBOC" || $jobtype eq "RESPONSE" || $jobtype eq "FC" ) {
      
      $fail |= compare_nuc();

      # DBOC is special, because it does not compute the wave function -- only check DBOC and break
      if ($jobtype eq "DBOC") {
        $fail |= compare_dboc();
	last SWITCH1;
      }
      
      # All computations in Psi3 start with an SCF run
      $fail |= compare_scf_energy();

      SWITCH2: {
        
          if ($wfn eq "CCSD")     { $fail |= compare_ccsd_energy(); last SWITCH2; }
          if ($wfn eq "CC2")      { $fail |= compare_cc2_energy(); last SWITCH2; }
	  if ($wfn eq "CCSD_T")   { $fail |= compare_ccsd_t_energy(); last SWITCH2; }
          if ($wfn eq "CC3")      { $fail |= compare_cc3_energy(); last SWITCH2; }
          if ($wfn eq "EOM_CC2")  { $fail |= compare_eomcc2_energy(); last SWITCH2; }
          if ($wfn eq "EOM_CCSD") { $fail |= compare_eomccsd_energy(); last SWITCH2; }
          if ($wfn eq "EOM_CC3")  { $fail |= compare_eomcc3_energy(); last SWITCH2; }
          if ($wfn eq "BCCD")     { $fail |= compare_bccd_energy(); last SWITCH2; }
          if ($wfn eq "BCCD_T")   { $fail |= compare_bccd_t_energy(); last SWITCH2; }
          if ($wfn eq "CASSCF")   { $fail |= compare_casscf_energy(); last SWITCH2; }
          if ($wfn eq "RASSCF")   { $fail |= compare_rasscf_energy(); last SWITCH2; }
          if ($wfn eq "DETCI")    { $fail |= compare_ci_energy(); last SWITCH2; }
          if ($wfn eq "CIS")      { $fail |= compare_cis_energy(); last SWITCH2; }
          if ($wfn eq "MP2" && $direct == 1)
                                  { $fail |= compare_direct_mp2_energy(); last SWITCH2; }
          if ($wfn eq "MP2" && $direct == 0)
                                  { $fail |= compare_mp2_energy(); last SWITCH2; }
          if ($wfn eq "MP2R12")   { $fail |= compare_mp2r12_energy(); last SWITCH2; }

      }
      
      if ($jobtype eq "SP" && $dertype eq "FIRST") {
        $fail |= compare_energy_file11($wfn);
        $fail |= compare_geom_file11($wfn);
        $fail |= compare_grad_file11($wfn);
      }
      
      if ( ($jobtype eq "FREQ" || $dertype eq "SP") && $dertype eq "SECOND") {
        $fail |= compare_harm_freq($wfn);
        $fail |= compare_harm_intensities($wfn);
      }

      if ($jobtype eq "FREQ" && $dertype eq "FIRST") {
        $fail |= compare_findif_freq($wfn);
      }

      if ($jobtype eq "FREQ" && $dertype eq "NONE") {
        $fail |= compare_findif_freq($wfn);
      }

      if ($jobtype eq "SYMM_FC" && $dertype eq "FIRST") {
        $fail |= compare_findif_symm_freq($wfn);
      }
      if ($jobtype eq "FC" && $dertype eq "FIRST") {
        $fail |= compare_findif_freq($wfn);
      }

      if ($jobtype eq "OEPROP") {
        $fail |= compare_mulliken_orb_pops();
        $fail |= compare_mulliken_ab_pops();
        $fail |= compare_mulliken_ga_pops();
        $fail |= compare_electric_dipole();
        $fail |= compare_elec_angmom();
        $fail |= compare_epef();
        $fail |= compare_edens();
        $fail |= compare_mvd();
      }
     
      if ($jobtype eq "SP" && $dertype eq "RESPONSE") {
        if ($wfn eq "CC2") {
          $fail |= compare_cclambda_overlap($wfn);
          $fail |= compare_cc_response($wfn);
        }
        if ($wfn eq "CCSD") {
          $fail |= compare_cclambda_overlap($wfn);
          $fail |= compare_cc_response($wfn);
        }
        if ($wfn eq "SCF") {
          $fail |= compare_scf_polar();
        }
      }

      if ($jobtype eq "OEPROP" && ($wfn eq "EOM_CC2" || $wfn eq "EOM_CCSD")) {
        $fail |= compare_eomcc_oeprop();
      }

      last SWITCH1;
    }

  }
  
  test_finished($fail,$interrupted);
}

##################################################################
#
# Following are utility functions
#
##################################################################
sub usage_notice
{
  printf "USAGE: $_[0] [options]\n";
  printf "       where options are:\n";
  printf "       -h    help\n";
  printf "       -c    do cleanup instead of testing\n";
  printf "       -u    exit cleanly even if execution fails\n";
  printf "       -q    run quietly, print out only the summary at the end\n";
}

sub test_started
{
  $test_name = get_test_name();
  $target = "$test_name.$PSITEST_TARGET_SUFFIX";
  open(RE, ">$target") || die "cannot open $target $!";
  close (RE);
  print_test_header();
}

sub pass_test
{
  $test_name = get_test_name();
  $target = "$test_name.$PSITEST_TARGET_SUFFIX";
  open(RE, ">>$target") || die "cannot open $target $!"; 
  printf RE "%-70s...PASSED\n", $_[0];
  close (RE);
}

sub fail_test
{
  $test_name = get_test_name();
  $target = "$test_name.$PSITEST_TARGET_SUFFIX";
  open(RE, ">>$target") || die "cannot open $target $!"; 
  printf RE "%-70s...FAILED\n", $_[0];
  close (RE);
}

sub print_test_header
{
  $test_name = get_test_name();
  $target = "$test_name.$PSITEST_TARGET_SUFFIX";
  open(RE, ">>$target") || die "cannot open $target $!"; 
  printf RE "$test_name:\n";
  close (RE);
}

sub test_finished
{
  my $fail = $_[0];
  my $interrupted = $_[1];

  $test_name = get_test_name();
  $target = "$test_name.$PSITEST_TARGET_SUFFIX";

  system("cat $target");

  if ($interrupted) {
    exit($fail);
  }
  else {
    exit(0);
  }
}

sub get_test_name
{
  use File::Basename;
  use Cwd;
  my $test_name = basename("$SRC_PATH");
  if ($test_name eq ".") {
    my $pwd = getcwd();
    $test_name = basename($pwd);
  }
  
  return $test_name;
}

sub compare_nuc
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_nuc($REF_FILE) - seek_nuc($TEST_FILE)) > $PSITEST_ENUCTOL) {
    fail_test("Nuclear repulsion energy"); $fail = 1;
  }
  else {
    pass_test("Nuclear repulsion energy");
  }
  
  return $fail;
}

sub compare_scf_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_scf($REF_FILE) - seek_scf($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("SCF energy"); $fail = 1;
  }
  else {
    pass_test("SCF energy");
  }
  
  return $fail;
}

sub compare_cc2_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";
                                                                                                              
  if(abs(seek_cc2($REF_FILE) - seek_cc2($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("CC2 energy"); $fail = 1;
  }
  else {
    pass_test("CC2 energy");
  }
                                                                                                              
  return $fail;
}
                                                                                                              
sub compare_ccsd_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_ccsd($REF_FILE) - seek_ccsd($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("CCSD energy"); $fail = 1;
  }
  else {
    pass_test("CCSD energy");
  }
  
  return $fail;
}

sub compare_ccsd_t_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_ccsd_t($REF_FILE) - seek_ccsd_t($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("CCSD(T) energy"); $fail = 1;
  }
  else {
    pass_test("CCSD(T) energy");
  }
  
  return $fail;
}

sub compare_cc3_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_cc3($REF_FILE) - seek_cc3($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("CC3 energy"); $fail = 1;
  }
  else {
    pass_test("CC3 energy");
  }
 
  return $fail;
}

sub compare_eomcc2_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @eom_ref = seek_eomcc($REF_FILE);
  @eom_test = seek_eomcc($TEST_FILE);

  if(!compare_arrays(\@eom_ref,\@eom_test,($#eom_ref+1),$PSITEST_EEOMTOL)) {
    fail_test("EOM-CC2 energy"); $fail = 1;
  }
  else {
    pass_test("EOM-CC2 energy");
  }
  
  return $fail;
}

sub compare_eomccsd_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @eom_ref = seek_eomcc($REF_FILE);
  @eom_test = seek_eomcc($TEST_FILE);

  if(!compare_arrays(\@eom_ref,\@eom_test,($#eom_ref+1),$PSITEST_EEOMTOL)) {
    fail_test("EOM-CCSD energy"); $fail = 1;
  }
  else {
    pass_test("EOM-CCSD energy");
  }
  
  return $fail;
}

sub compare_eomcc3_energy
{
  my $fail = 0;  
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";   

  @eom_ref = seek_eomcc($REF_FILE);   
  @eom_test = seek_eomcc($TEST_FILE);

  if(!compare_arrays(\@eom_ref,\@eom_test,($#eom_ref+1),$PSITEST_EEOMTOL)) {
    fail_test("EOM-CC3 energy"); $fail = 1;
  }  
  else {
    pass_test("EOM-CC3 energy");
  }       
  return $fail;
}         


sub compare_bccd_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_bccd($REF_FILE) - seek_bccd($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("B-CCD energy"); $fail = 1;
  }
  else {
    pass_test("B-CCD energy");
  }
  
  return $fail;
}

sub compare_bccd_t_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_ccsd_t($REF_FILE) - seek_ccsd_t($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("CCSD(T) energy"); $fail = 1;
  }
  else {
    pass_test("CCSD(T) energy");
  }
  
  return $fail;
}

sub compare_cclambda_overlap
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_lambda($REF_FILE) - seek_lambda($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("$wfn Lambda Overlap"); $fail = 1;
  }
  else {
    pass_test("$wfn Lambda Overlap");
  }
  
  return $fail;
}

sub compare_scf_polar
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @polar_ref = seek_scf_polar($REF_FILE);
  @polar_test = seek_scf_polar($TEST_FILE);

  if(!compare_arrays(\@polar_ref,\@polar_test,($#polar_ref+1),$PSITEST_POLARTOL)) {
    fail_test("SCF Polarizability"); $fail = 1;
  }
  else {
    pass_test("SCF Polarizability");
  }
  
  return $fail;
}

sub compare_cc_response
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  # Grab field frequencies
  $nfreqs = seek_num_cc_response_freqs($TEST_FILE);
#  printf "Number of frequencies = %d\n", $nfreqs;
  @polar_freqs = seek_cc_response_freqs($TEST_FILE);

  # What response property are we testing?
  $property = seek_cc_response_property($TEST_FILE);
#  printf "Response property = %s\n", $property;

  if($property eq "ROTATION") {
      # Length, velocity, or both?
      $gauge = seek_cc_response_gauge($TEST_FILE);
      if($gauge eq "BOTH") {
	  for($i=0; $i < $nfreqs; $i++) {

	      $alpha_ref = seek_cc_optrot($REF_FILE, $polar_freqs[$i], "LENGTH");
	      $alpha_test = seek_cc_optrot($TEST_FILE, $polar_freqs[$i], "LENGTH");
	      if(abs($alpha_ref - $alpha_test) >  $PSITEST_OPTROTTOL) {
		  fail_test("$wfn Optical Rotation (LENGTH) @ $polar_freqs[$i]"); $fail = 1;
	      }
	      else {
		  pass_test("$wfn Optical Rotation (LENGTH) @ $polar_freqs[$i]");
	      }

	      $alpha_ref = seek_cc_optrot($REF_FILE, $polar_freqs[$i], "VELOCITY");
	      $alpha_test = seek_cc_optrot($TEST_FILE, $polar_freqs[$i], "VELOCITY");
	      if(abs($alpha_ref - $alpha_test) >  $PSITEST_OPTROTTOL) {
		  fail_test("$wfn Optical Rotation (VELOCITY) @ $polar_freqs[$i]"); $fail = 1;
	      }
	      else {
		  pass_test("$wfn Optical Rotation (VELOCITY) @ $polar_freqs[$i]");
	      }

	      @vector_ref = seek_cc_delta($REF_FILE, $polar_freqs[$i]);
	      @vector_test = seek_cc_delta($TEST_FILE, $polar_freqs[$i]);
	      if(!compare_arrays(\@vector_ref, \@vector_test, ($#vector_test+1), $PSITEST_OPTROTTOL)) {
		  fail_test("$wfn Optical Rotation Origin-Dependence Vector @ $polar_freqs[$i]"); $fail = 1;
	      }
	      else {
		  pass_test("$wfn Optical Rotation Origin-Dependence Vector @ $polar_freqs[$i]");
	      }
	  }
      }
      else {
	  for($i=0; $i < $nfreqs; $i++) {

	      $alpha_ref = seek_cc_optrot($REF_FILE, $polar_freqs[$i], $gauge);
	      $alpha_test = seek_cc_optrot($TEST_FILE, $polar_freqs[$i], $gauge);
	      if(abs($alpha_ref - $alpha_test) >  $PSITEST_OPTROTTOL) {
		  fail_test("$wfn Optical Rotation ($gauge) @ $polar_freqs[$i]"); $fail = 1;
	      }
	      else {
		  pass_test("$wfn Optical Rotation ($gauge) @ $polar_freqs[$i]");
	      }
	  }
      }

  }
  elsif($property eq "POLARIZABILITY") {
      for($i=0; $i < $nfreqs; $i++) {
	  $alpha_ref = seek_cc_polar($REF_FILE, $polar_freqs[$i], $gauge);
	  $alpha_test = seek_cc_polar($TEST_FILE, $polar_freqs[$i], $gauge);
	  if(abs($alpha_ref - $alpha_test) >  $PSITEST_POLARTOL) {
	      fail_test("$wfn Polarizability @ $polar_freqs[$i]"); $fail = 1;
	  }
	  else {
	      pass_test("$wfn Polarizability @ $polar_freqs[$i]");
	  }
      }
  }

  return $fail;
}

sub compare_eomcc_oeprop
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  $NSREF = seek_nstates($REF_FILE);
  $NSTEST = seek_nstates($TEST_FILE);

  $LABEL = "Number of States";
  if($NSREF != $NSTEST) {
    fail_test("$LABEL"); $fail = 1;
  }  
  else {
    pass_test("$LABEL");
    $NSTATES = $NSREF;
  } 

  @int_ref = seek_excitation_energy($REF_FILE,"OS       RS",$NSTATES);
  @int_test = seek_excitation_energy($TEST_FILE,"OS       RS",$NSTATES);

  $LABEL = "Excitation Energy";
  if(!compare_arrays(\@int_ref, \@int_test, $NSTATES, $TOL)) {
    fail_test("$LABEL"); $fail = 1;
  }
  else {
    pass_test("$LABEL");
  }

  @int_ref = seek_osc_str($REF_FILE,"OS       RS",$NSTATES);
  @int_test = seek_osc_str($TEST_FILE,"OS       RS",$NSTATES);

  $LABEL = "Oscillator Strength";
  if(!compare_arrays(\@int_ref, \@int_test, $NSTATES, $TTOL)) {
    fail_test("$LABEL"); $fail = 1;
  }
  else {
    pass_test("$LABEL");
  }

  @int_ref = seek_rot_str($REF_FILE,"OS       RS",$NSTATES);
  @int_test = seek_rot_str($TEST_FILE,"OS       RS",$NSTATES);

  $LABEL = "Rotational Strength";
  if(!compare_arrays(\@int_ref, \@int_test, $NSTATES, $TTOL)) {
    fail_test("$LABEL"); $fail = 1;
  }
  else {
    pass_test("$LABEL");
  }

  return $fail;
}

sub compare_casscf_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_casscf($REF_FILE) - seek_casscf($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("CASSCF energy"); $fail = 1;
  }
  else {
    pass_test("CASSCF energy");
  }
  
  return $fail;
}

sub compare_rasscf_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_rasscf($REF_FILE) - seek_rasscf($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("RASSCF energy"); $fail = 1;
  }
  else {
    pass_test("RASSCF energy");
  }
  
  return $fail;
}

sub compare_cis_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @cis_ref = seek_cis($REF_FILE);
  @cis_test = seek_cis($TEST_FILE);

  if(!compare_arrays(\@cis_ref,\@cis_test,($#cis_ref+1),$PSITEST_ETOL)) {
    fail_test("CIS Energies"); $fail = 1;
  }
  else {
    pass_test("CIS Energies");
  }
  
  return $fail;
}

sub compare_ci_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_ci($REF_FILE) - seek_ci($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("CI energy"); $fail = 1;
  }
  else {
    pass_test("CI energy");
  }

  if($fail == 0 && dip_check($REF_FILE) == 1) {
    @ci_dip_ref = seek_ci_dip($REF_FILE);
    @ci_dip_test = seek_ci_dip($TEST_FILE);
    
    $size_ci_dip_ref = @ci_dip_ref;
    $size_ci_dip_test = @ci_dip_test;

    if($size_ci_dip_ref != $size_ci_dip_test) {
      fail_test("CI dipole"); $fail = 1;      
    }
    else {
      for(my $i=0; $i < $size_ci_dip_ref; $i++) {
        if(abs($ci_dip_ref[$i] - $ci_dip_test[$i]) > $PSITEST_CIDIPTOL) {
          $j = $i+1; 
          fail_test("CI dipole $j of $size_ci_dip_ref"); $fail = 1;
        }
        else {
          $j = $i+1;
          pass_test("CI dipole $j of $size_ci_dip_ref");
        }
      }
    }
  }
  
  return $fail;
}

sub dip_check
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/\|mu\|/) {
      close(OUT);
      return 1;
    }
  }

  close(OUT);
  return 0;
}

sub seek_ci_dip
{
  my $i = 0;
  my @ci_dip;
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/\|mu\|/) {
      @data = split(/ +/, $_);
      $ci_dip[$i] = $data[4];
      $i++;
    }
  }
  
  return @ci_dip;
  close(OUT);
}

sub compare_dboc
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_dboc($REF_FILE) - seek_dboc($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("DBOC"); $fail = 1;
  }
  else {
    pass_test("DBOC");
  }
  
  return $fail;
}

sub compare_mp2_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_mp2($REF_FILE) - seek_mp2($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("MP2 Energy"); $fail = 1;
  }
  else {
    pass_test("MP2 Energy");
  }

  if($fail == 0 && scs_check($REF_FILE) == 1) {
    
    if(abs(seek_scs_mp2($REF_FILE) - seek_scs_mp2($TEST_FILE)) > $PSITEST_ETOL) {
      fail_test("SCS-MP2 Energy"); $fail = 1;
    }
    else {
      pass_test("SCS-MP2 Energy");
    }
  } 
 
  return $fail;
}

sub scs_check
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/SCS\s+=\s+TRUE/) {
      close(OUT);
      return 1;
    }
  }
  
  close(OUT);
  return 0;
}

sub compare_direct_mp2_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_mp2_direct($REF_FILE) - seek_mp2_direct($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("Direct MP2 Energy"); $fail = 1;
  }
  else {
    pass_test("Direct MP2 Energy");
  }
  
  return $fail;
}

sub compare_mulliken_orb_pops
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @gop_ref = seek_mulliken_gop($REF_FILE);
  @gop_test = seek_mulliken_gop($TEST_FILE);

  if(!compare_arrays(\@gop_ref,\@gop_test,($#gop_ref+1),$PSITEST_MPOPTOL)) {
    fail_test("Gross Orbital Populations"); $fail = 1;
  }
  else {
    pass_test("Gross Orbital Populations");
  }
  
  return $fail;
}

sub compare_mulliken_ab_pops
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @abp_ref = seek_mulliken_abp($REF_FILE);
  @abp_test = seek_mulliken_abp($TEST_FILE);

  if(!compare_arrays(\@abp_ref,\@abp_test,($#abp_ref+1),$PSITEST_MPOPTOL)) {
    fail_test("Atomic Bond Populations"); $fail = 1;
  }
  else {
    pass_test("Atomic Bond Populations");
  }
  
  return $fail;
}

sub compare_mulliken_ga_pops
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @apnc_ref = seek_mulliken_apnc($REF_FILE);
  @apnc_test = seek_mulliken_apnc($TEST_FILE);

  if(!compare_arrays(\@apnc_ref,\@apnc_test,($#apnc_ref+1),$PSITEST_MPOPTOL)) {
    fail_test("Gross Atomic Populations and Net Charges"); $fail = 1;
  }
  else {
    pass_test("Gross Atomic Populations and Net Charges");
  }
  
  return $fail;
}

sub compare_electric_dipole
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @edipole_ref = seek_dipole($REF_FILE);
  @edipole_test = seek_dipole($TEST_FILE);

  if(!compare_arrays(\@edipole_ref,\@edipole_test,($#edipole_ref+1),$PSITEST_MPOPTOL)) {
    fail_test("Electric Dipole Moment"); $fail = 1;
  }
  else {
    pass_test("Electric Dipole Moment");
  }
  
  return $fail;
}

sub compare_elec_angmom
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @eangmom_ref = seek_angmom($REF_FILE);
  @eangmom_test = seek_angmom($TEST_FILE);

  if(!compare_arrays(\@eangmom_ref,\@eangmom_test,($#eangmom_ref+1),$PSITEST_MPOPTOL)) {
    fail_test("Electric Angular Momentum"); $fail = 1;
  }
  else {
    pass_test("Electric Angular Momentum");
  }
  
  return $fail;
}

sub compare_epef
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @epef_ref = seek_epef($REF_FILE);
  @epef_test = seek_epef($TEST_FILE);

  if(!compare_arrays(\@epef_ref,\@epef_test,($#epef_ref+1),$PSITEST_MPOPTOL)) {
    fail_test("Electrostatic Potential and Electric Field"); $fail = 1;
  }
  else {
    pass_test("Electrostatic Potential and Electric Field");
  }
  
  return $fail;
}

sub compare_edens
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  @edensity_ref = seek_edensity($REF_FILE);
  @edensity_test = seek_edensity($TEST_FILE);

  if(!compare_arrays(\@edensity_ref,\@edensity_test,($#edensity_ref+1),$PSITEST_MPOPTOL)) {
    fail_test("Electron Density"); $fail = 1;
  }
  else {
    pass_test("Electron Density");
  }
  
  return $fail;
}

sub compare_mvd
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_mvd($REF_FILE) - seek_mvd($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("One-Electron Relativistic Correction (MVD)"); $fail = 1;
  }
  else {
    pass_test("One-Electron Relativistic Correction (MVD)");
  }
  
  return $fail;
}

sub compare_mp2r12_energy
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  if(abs(seek_mp2r12($REF_FILE) - seek_mp2r12($TEST_FILE)) > $PSITEST_ETOL) {
    fail_test("MP2-R12 Energy"); $fail = 1;
  }
  else {
    pass_test("MP2-R12 Energy");
  }
  
  return $fail;
}

sub compare_rhf_stability
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  my $nirreps = seek_nirreps($REF_FILE);
  my $label = "RHF->RHF";
  @stab_ref = seek_stab($REF_FILE, $label, $PSITEST_DEFAULT_NSTAB, $nirreps);
  @stab_test = seek_stab($TEST_FILE, $label, $PSITEST_DEFAULT_NSTAB, $nirreps);

  if(!compare_arrays(\@stab_ref,\@stab_test,($#stab_ref+1),$PSITEST_STABTOL)) {
    fail_test("$label Stability"); $fail = 1;
  }
  else {
    pass_test("$label Stability");
  }

  my $label = "RHF->UHF";
  @stab_ref = seek_stab($REF_FILE, $label, $PSITEST_DEFAULT_NSTAB, $nirreps);
  @stab_test = seek_stab($TEST_FILE, $label, $PSITEST_DEFAULT_NSTAB, $nirreps);

  if(!compare_arrays(\@stab_ref,\@stab_test,($#stab_ref+1),$PSITEST_STABTOL)) {
    fail_test("$label Stability"); $fail = 1;
  }
  else {
    pass_test("$label Stability");
  }

  return $fail;
}

sub compare_rohf_stability
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  my $nirreps = seek_nirreps($REF_FILE);
  my $label = "ROHF->ROHF";
  @stab_ref = seek_stab($REF_FILE, $label, $PSITEST_DEFAULT_NSTAB, $nirreps);
  @stab_test = seek_stab($TEST_FILE, $label, $PSITEST_DEFAULT_NSTAB, $nirreps);

  if(!compare_arrays(\@stab_ref,\@stab_test,($#stab_ref+1),$PSITEST_STABTOL)) {
    fail_test("$label Stability"); $fail = 1;
  }
  else {
    pass_test("$label Stability");
  }

  return $fail;
}

sub compare_uhf_stability
{
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  my $nirreps = seek_nirreps($REF_FILE);
  my $label = "UHF->UHF";
  @stab_ref = seek_stab($REF_FILE, $label, $PSITEST_DEFAULT_NSTAB, $nirreps);
  @stab_test = seek_stab($TEST_FILE, $label, $PSITEST_DEFAULT_NSTAB, $nirreps);

  if(!compare_arrays(\@stab_ref,\@stab_test,($#stab_ref+1),$PSITEST_STABTOL)) {
    fail_test("$label Stability"); $fail = 1;
  }
  else {
    pass_test("$label Stability");
  }

  return $fail;
}

sub compare_harm_freq
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  my $ndof = seek_ndof($REF_FILE);
  @freq_ref = seek_anal_freq($REF_FILE,"Harmonic Frequency",$ndof);
  @freq_test = seek_anal_freq($TEST_FILE,"Harmonic Frequency",$ndof);

  if(!compare_arrays(\@freq_ref,\@freq_test,($#freq_ref+1),$PSITEST_HTOL)) {
    fail_test("$wfn Frequencies"); $fail = 1;
  }
  else {
    pass_test("$wfn Frequencies");
  }

  return $fail;
}

sub compare_harm_intensities
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  my $ndof = seek_ndof($REF_FILE);
  @int_ref = seek_int($REF_FILE,"Harmonic Frequency",$ndof);
  @int_test = seek_int($TEST_FILE,"Harmonic Frequency",$ndof);

  if(!compare_arrays(\@int_ref,\@int_test,($#int_ref+1),$PSITEST_HTOL)) {
    fail_test("$wfn Intensities"); $fail = 1;
  }
  else {
    pass_test("$wfn Intensities");
  }

  return $fail;
}

sub compare_energy_file11
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/file11.ref";
  my $TEST_FILE = "$PSITEST_PREFIX.file11.dat";

  if(abs(seek_energy_file11($REF_FILE,$wfn) - seek_energy_file11($TEST_FILE,$wfn)) > $PSITEST_ETOL) {
    fail_test("$wfn energy"); $fail = 1;
  }
  else {
    pass_test("$wfn energy");
  }
  
  return $fail;
}

sub compare_geom_file11
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/file11.ref";
  my $TEST_FILE = "$PSITEST_PREFIX.file11.dat";

  my @geom_ref = seek_geom_file11($REF_FILE, $wfn);
  my @geom_test = seek_geom_file11($TEST_FILE, $wfn);
  if(!compare_arrays(\@geom_ref, \@geom_test, ($#geom_ref+1), $PSITEST_GEOMTOL)) {
    fail_test("$wfn Geometry"); $fail = 1;
  }
  else {
    pass_test("$wfn Geometry");
  }
    
  return $fail;
}

sub compare_grad_file11
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/file11.ref";
  my $TEST_FILE = "$PSITEST_PREFIX.file11.dat";

  my @grad_ref = seek_grad_file11($REF_FILE, $wfn);
  my @grad_test = seek_grad_file11($TEST_FILE, $wfn);  
  if(!compare_arrays(\@grad_ref, \@grad_test, ($#grad_ref+1), $PSITEST_GTOL)) {
    fail_test("$wfn Gradient"); $fail = 1;
  }
  else {
    pass_test("$wfn Gradient");
  }
    
  return $fail;
}

sub compare_findif_freq
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";
  my $ndof = seek_ndof($REF_FILE);

  my @freq_ref = seek_findif_freq($REF_FILE, "Harmonic Vibrational Frequencies", $ndof);
  my @freq_test = seek_findif_freq($TEST_FILE, "Harmonic Vibrational Frequencies", $ndof);  
  if(!compare_arrays(\@freq_ref, \@freq_test, ($#freq_ref+1), $PSITEST_HTOL)) {
    fail_test("$wfn Frequencies"); $fail = 1;
  }
  else {
    pass_test("$wfn Frequencies");
  }
    
  return $fail;
}

sub compare_findif_symm_freq
{
  my $wfn = $_[0];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";
  my $ndof = seek_ndof_symm($REF_FILE);

  my @freq_ref = seek_findif_freq($REF_FILE, "Harmonic Vibrational Frequencies", $ndof);
  my @freq_test = seek_findif_freq($TEST_FILE, "Harmonic Vibrational Frequencies", $ndof);  
  if(!compare_arrays(\@freq_ref, \@freq_test, ($#freq_ref+1), $PSITEST_HTOL)) {
    fail_test("$wfn Frequencies"); $fail = 1;
  }
  else {
    pass_test("$wfn Frequencies");
  }
    
  return $fail;
}

sub compare_pair_energies
{
  my $wfn = $_[0];
  my $spincase = $_[1];
  my $fail = 0;
  my $REF_FILE = "$SRC_PATH/output.ref";
  my $TEST_FILE = "output.dat";

  my @epair_s_ref = seek_pair_energies($REF_FILE, $wfn, $spincase);
  my @epair_s_test = seek_pair_energies($TEST_FILE, $wfn, $spincase);  
  if(!compare_arrays(\@epair_s_ref, \@epair_s_test, ($#epair_s_ref+1), $PSITEST_ETOL)) {
    fail_test("$wfn $spincase Pair Energies"); $fail = 1;
  }
  else {
    pass_test("$wfn $spincase Pair Energies");
  }
    
  return $fail;
}

sub seek_nirreps
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Number of irr. rep.      =/) {
      @data = split(/[ \t]+/, $_);
      my $nirreps = $data[6];
      return $nirreps;
    }
  }
  close(OUT);

  printf "Error: Could not find number of irreducible representations in $_[0].\n";
  exit 1;
}

sub seek_natoms
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Number of atoms          =/) {
      @data = split(/[ \t]+/, $_);
      my $natoms = $data[5];
      return $natoms;
    }
  }
  close(OUT);

  printf "Error: Could not find number of atoms in $_[0].\n";
  exit 1;
}

sub seek_linear
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/    It is a linear molecule./) {
      return 1;
    }
  }
  close(OUT);

  return 0;
}

sub seek_ndof
{
  my $natoms = seek_natoms($_[0]);
  my $is_linear = seek_linear($_[0]);
  my $ndof = 3*$natoms - 6;
  if ($is_linear == 1) {
    $ndof += 1;
  }
  return $ndof;
}

sub seek_ndof_symm
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/ salcs of this irrep/) {
      my @data = split(/ +/, $_);
      my $ndof_symm = $data[1];
      return $ndof_symm;
    }
  }
  close(OUT);

  printf "Error: Could not find the number of symmetric degrees of freedom in $_[0].\n";
  exit 1;
}

sub seek_nuc
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Nuclear Repulsion Energy \(a.u.\) =/) {
      @data = split(/ +/, $_);
      $nuc = $data[6];
      return $nuc;
    }
  }
  close(OUT);

  printf "Error: Could not find nuclear repulsion energy in $_[0].\n";
  exit 1;
}

sub seek_scf
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/SCF total energy/) {
      @data = split(/ +/, $_);
      $scf = $data[5];
      return $scf;
    }
  }
  close(OUT);

  printf "Error: Could not find SCF energy in $_[0].\n";
  exit 1;
}

sub seek_mp2
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/\s+MP2 total energy/) {
      @data = split(/ +/, $_);
      $mp2 = $data[4];
      return $mp2;
    }
  }
  close(OUT);

  printf "Error: Could not find MP2 energy in $_[0].\n";
  exit 1;
}

sub seek_mp2_direct
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Total MBPT/) {
      @data = split(/ +/, $_);
      $mp2 = $data[5];
      return $mp2;
    }
  }
  close(OUT);

  printf "Error: Could not find MP2 energy in $_[0].\n";
  exit 1;
}

sub seek_scs_mp2
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/SCS-MP2 total energy/) {
      @data = split(/ +/, $_);
      $scs_mp2 = $data[4];
      return $scs_mp2;
    }
  }
  close(OUT);

  printf "Error: Could not find SCS-MP2 energy in $_[0].\n";
  exit 1;
}

sub seek_mp2r12
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/MBPT\(2\)-R12/) {
      @data = split(/ +/, $_);
      $mp2r12 = $data[3];
      return $mp2r12;
    }
  }
  close(OUT);

  printf "Error: Could not find MBPT(2)-R12 energy in $_[0].\n";
  exit 1;
}

# find the MP2 energy in the MP2-R12 output
sub seek_mp2r12_mp2
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/MBPT\(2\) Energy/) {
      @data = split(/ +/, $_);
      $mp2 = $data[3];
      return $mp2;
    }
  }
  close(OUT);

  printf "Error: Could not find MBPT(2) energy in $_[0].\n";
  exit 1;
}

sub seek_cc2
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Total CC2 energy/) {
      @data = split(/ +/, $_);
      $cc2 = $data[4];
      return $cc2;
    }
  }
  close(OUT);
                                                                                                              
  printf "Error: Could not find CC2 energy in $_[0].\n";
  exit 1;
}
                                                                                                              
sub seek_ccsd
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Total CCSD energy/) {
      @data = split(/ +/, $_);
      $ccsd = $data[4];
      return $ccsd;
    }
  }
  close(OUT);

  printf "Error: Could not find CCSD energy in $_[0].\n";
  exit 1;
}

sub seek_ccsd_t
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Total CCSD\(T\) energy/) {
      @data = split(/ +/, $_);
      $ccsd_t = $data[4];
      return $ccsd_t;
    }
  }
  close(OUT);

  printf "Error: Could not find CCSD(T) energy in $_[0].\n";
  exit 1;
}

sub seek_cc3
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Total CC3 energy/) {
      @data = split(/ +/, $_);
      $ccsd = $data[4];
      return $ccsd;
    }
  }
  close(OUT);

  printf "Error: Could not find CC3 energy in $_[0].\n";
  exit 1;
}

sub seek_bccd
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "Total CCSD energy";
  $linenum = 0;
  $lastiter = 0;

  foreach $line (@datafile) {
    if ($line =~ m/$match/) {
      $lastiter = $linenum;
    }
    $linenum++;
  }

  @line = split (/ +/, $datafile[$lastiter]);
  $bccd = $line[5];

  if($bccd != 0.0) {
    return $bccd;
  }

  printf "Error: Could not find B-CCD energy in $_[0].\n";
  exit 1;
}

sub seek_lambda
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Overlap <L|e^T> =/) {
      @data = split(/ +/, $_);
      $lambda = $data[3];
      return $lambda;
    }
  }
  close(OUT);

  printf "Error: Could not find CCSD Lambda Overlap in $_[0].\n";
  exit 1;
}

sub seek_casscf
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";

#Added to handle David's new CASSCF output format
  while (<OUT>) {
      if (/Final CASSCF Energy/) {
	  @data = split(/=/, $_);
	  $casscf = $data[2];
	  return $casscf;
      }
  }

  printf "Error: Could not find CASSCF energy in $_[0].\n";
  exit 1;
}

sub seek_rasscf
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";

#Added to handle David's new RASSCF output format
  while (<OUT>) {
      if (/Final RASSCF Energy/) {
	  @data = split(/=/, $_);
	  $casscf = $data[2];
	  return $casscf;
      }
  }

  printf "Error: Could not find RASSCF energy in $_[0].\n";
  exit 1;
}
  
sub seek_ci
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while (<OUT>) {
    if (/ROOT 1/) {
    @data = split(/ +/, $_);
    $ci = $data[4];
    }
  }

  if($ci != 0.0) {
    return $ci;
  }

  printf "Error: Could not find CI energy in $_[0].\n";
  exit 1;
}
			  
sub seek_energy_file11
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $linenum = 0;
  $lasiter = 0;

  foreach $line (@datafile) {
    if($line =~ m/$match/) {
      $lastiter = $linenum;
    }
    $linenum++;
  }

  @line = split(/ +/, $datafile[$lastiter+1]);
  $energy = $line[2];

  if($energy != 0.0) {
    return $energy;
  }

  printf "Error: Could not find $_[1] energy in $_[0].\n";
  exit 1;
}

sub seek_natom_file11
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $linenum = 0;
  $lasiter = 0;

  foreach $line (@datafile) {
    if($line =~ m/$match/) {
      $lastiter = $linenum;
    }
    $linenum++;
  }

  @line = split(/ +/, $datafile[$lastiter+1]);
  $natom = $line[1];

  if($natom != 0) {
    return $natom;
  }

  printf "Error: Could not find value of natom in $_[0].\n";
  exit 1;
}

sub seek_geom_file11
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $linenum = 0;
  $lasiter = 0;
  $foundit = 0;

  foreach $line (@datafile) {
    if($line =~ m/$match/) {
      $foundit = 1;
      $lastiter = $linenum;
    }
    $linenum++;
  }

  @line = split(/ +/, $datafile[$lastiter+1]);
  $natom = $line[1];

  for($i=0; $i < $natom; $i++) {
    @line = split(/ +/, $datafile[$lastiter+2+$i]);
    $geom[3*$i] = $line[2];
    $geom[3*$i+1] = $line[3];
    $geom[3*$i+2] = $line[4];
  }

  if($foundit != 0) {
    return @geom;
  }

  printf "Error: Could not find $_[1] geom in $_[0].\n";
  exit 1;
}

sub seek_grad_file11
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $linenum = 0;
  $lasiter = 0;
  $foundit = 0;

  foreach $line (@datafile) {
    if($line =~ m/$match/) {
      $foundit = 1;
      $lastiter = $linenum;
    }
    $linenum++;
  }

  @line = split(/ +/, $datafile[$lastiter+1]);
  $natom = $line[1];

  for($i=0; $i < $natom; $i++) {
    @line = split(/ +/, $datafile[$lastiter+2+$natom+$i]);
    $grad[3*$i] = $line[1];
    $grad[3*$i+1] = $line[2];
    $grad[3*$i+2] = $line[3];
  }

  if($foundit != 0) {
    return @grad;
  }

  printf "Error: Could not find $_[1] grad in $_[0].\n";
  exit 1;
}

sub seek_findif_freq
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $ndof = "$_[2]";
  $j=0;
  $linenum=0;
  foreach $line (@datafile) {
    $linenum++;
    if ($line =~ m/$match/) {
      while ($j<$ndof) {
        @test = split (/ +/,$datafile[$linenum+1+$j]);
        $freq[$j] = $test[2];
        $j++;
      }
    }
  }

  $OK = 1;
  for($i=0; $i < $ndof; $i++) {
#    printf "%d %6.1f\n", $i, $freq[$i];
    if($freq[$i] == 0.0 || $freq[$i] > 6000) {
      $OK = 0; 
    }
  }
  
  if($OK && $ndof > 0) {
    return @freq;
  }

  printf "Error: Check $_[1] in $_[0].\n";
  exit 1;
}

sub seek_anal_freq
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $ndof = "$_[2]";
  $j=0;
  $linenum=0;
  foreach $line (@datafile) {
    $linenum++;
    if ($line =~ m/$match/) {
      while ($j<$ndof) {
        @test = split (/ +/,$datafile[$linenum+2+$j]);
        $freq[$j] = $test[2];
        $j++;
      }
    }
  }

  $OK = 1;
  for($i=0; $i < $ndof; $i++) {
#    printf "%d %6.1f\n", $i, $freq[$i];
    if($freq[$i] < 0.0 || $freq[$i] > 6000) {
      $OK = 0; 
    }
  }
  
  if($OK && $ndof > 0) {
    return @freq;
  }

  printf "Error: Check $_[1] in $_[0].\n";
  exit 1;
}

sub seek_int
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  # set up some initial values to be overwritten 
  for($i=0; $i < $ndof; $i++) {
    $int[$i] = -1.0;
  }

  $match = "$_[1]";
  $ndof = "$_[2]";
  $j=0;
  $linenum=0;
  foreach $line (@datafile) {
    $linenum++;
    if ($line =~ m/$match/) {
      while ($j<$ndof) {
        @test = split (/ +/,$datafile[$linenum+2+$j]);
        $int[$j] = $test[3];
        $j++;
      }
    }
  }

  $OK = 1;
  for($i=0; $i < $ndof; $i++) {
    if($int[$i] < 0.0) {
      $OK = 0;
    }
  }

  if($OK && $ndof > 0) {
    return @int;
  }

  printf "Error: Check $_[1] in $_[0].\n";
  exit 1;
}

sub seek_eomcc
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $linenum=0;
  $eval = 0;
  foreach $line (@datafile) {
    $linenum++;
    if ($line =~ m/EOM State/) {
      @test = split (/ +/,$datafile[$linenum-1]);
      $evals[$eval] = $test[6];
      $eval++;
    }
  }

  if($eval != 0) {
    return @evals;
  }

  printf "Error: Could not find EOM energies in $_[0].\n";
  exit 1;
}

sub seek_cis
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);
  
  $linenum=0;
  $eval = 0;
  foreach $line (@datafile) {
    $linenum++;
    if ($line =~ m/CIS State/) {
      @test = split (/ +/,$datafile[$linenum-1]);
      $evals[$eval] = $test[4];
      $eval++;
    }
  }
    
  if($eval != 0) {
    return @evals;
  }
    
  printf "Error: Could not find CIS energies in $_[0].\n";
  exit 1;
}


sub seek_stab
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $num_evals = $_[2];
  $num_symms = $_[3];
  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/$match/) {
      $start = $linenum;
    }
    $linenum++;
  }

  for($i=0; $i < $num_evals; $i++) {
    @line = split(/ +/, $datafile[$start+4+$i]);
    for($j=0; $j < $num_symms; $j++) {
      $stab[$num_symms*$i+$j] = $line[$j+2];
    }
  }

  if($start != 0) {
    return @stab;
  }

  printf "Error: Could not find $_[1] stability eigenvalues in $_[0].\n";
  exit 1;
}

sub seek_scf_polar
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/Hartree-Fock Electric Polarizability Tensor/) {
      $start = $linenum;
    }
    $linenum++;
  }

  for($i=0; $i < 3; $i++) {
    @line = split(/ +/, $datafile[$start+5+$i]);
    for($j=0; $j < 3; $j++) {
      $polar[3*$i+$j] = $line[$j+2];
    }
  }

  if($start != 0) {
    return @polar;
  }

  printf "Error: Could not find SCF polarizability tensor in $_[0].\n";
  exit 1;
}

sub seek_cc_response_property
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  foreach $line (@datafile) {
      if ($line =~ m/Property/) {
	  @data = split(/ +/, $line);
	  chomp($data[2]);
	  return $data[2];
      }
  }

  printf "Error: Could not determine type of response property in $_[0],\n";
  exit 1;
}

sub seek_cc_response_gauge
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  foreach $line (@datafile) {
      if ($line =~ m/Gauge/) {
	  @data = split(/ +/, $line);
	  chomp($data[2]);
	  return $data[2];
      }
  }

  printf "Error: Could not determine type of gauge in $_[0],\n";
  exit 1;
}

sub seek_num_cc_response_freqs
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $value = 0;
  foreach $line (@datafile) {
      if ($line =~ m/Applied field/) {
	  $value++;
      }
  }

  if($value) {
      return $value;
  }

  printf "Error: Could not find CC response frequencies in $_[0].\n";
  exit 1;
}

sub seek_cc_response_freqs
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $value = 0;
  foreach $line (@datafile) {
    if ($line =~ m/Applied field/) {
	@data = split(/ +/, $line);
	chomp($data[4]);
	$freqs[$value] = $data[4];
	$value++;
    }
  }

  if($value) {
      return @freqs;
  }

  printf "Error: Could not find CC response frequencies in $_[0].\n";
  exit 1;
}

sub seek_cc_optrot
{
  $freq = $_[1];
  $gauge = $_[2];

  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $linenum = 0;
  foreach $line (@datafile) {
      if($gauge eq "LENGTH") {
	  if(($line =~ m/using length-gauge/)) {
	      @data = split(/ +/, $datafile[$linenum+1]);
	      if($data[0] =~ m/$freq/) {
		  return $data[2];
	      }
	  }
      }
      elsif($gauge eq "VELOCITY") {
	  if(($line =~ m/using velocity-gauge/)) {
	      @data = split(/ +/, $datafile[$linenum+1]);
	      if($data[0] =~ m/$freq/) {
		  return $data[2];
	      }
	  }
      }
      $linenum++;
  }

  printf "Error: Could not find CC optical rotation in $_[0].\n";
  exit 1;
}

sub seek_cc_polar
{
  $freq = $_[1];
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $linenum = 0;
  foreach $line (@datafile) {
      if($line =~ m/alpha_\($freq\)/) {
	  @data = split(/ +/, $line);
	  return $data[2];
      }
  }

  printf "Error: Could not find CC polarizability in $_[0].\n";
  exit 1;
}

sub seek_cc_delta
{
  $freq = $_[1];

  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $linenum = 0;
  foreach $line (@datafile) {
      if($line =~ m/vector for length-gauge/) {
#	  printf "%s\n", $datafile[$linenum+1];
	  @data = split(/ +/, $datafile[$linenum+1]);
	  $vector[0] = $data[3];
	  $vector[1] = $data[6];
	  $vector[2] = $data[9];
#	  printf "%5.2f %5.2f %5.2f\n", $vector[0], $vector[1], $vector[2];
	  return @vector;
      }
      $linenum++;
  }

  printf "Error: Could not find CC origin-dependence vector in $_[0].\n";
  exit 1;
}

sub seek_dboc
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/E\(DBOC\)/) {
      @data = split(/ +/, $_);
      $Edboc = $data[3];
      return $Edboc;
    }
  } 
  close(OUT);
  
  printf "Error: Could not find DBOC in $_[0].\n";
  exit 1;
}   

sub seek_mulliken_gop
{   
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/# of atomic orbitals/) {
      @data = split(/[ \t]+/, $_);
      $nao = $data[6];
    }
  }
  close (OUT);

  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close (OUT);

  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/-Gross orbital populations/) {
      $start = $linenum;
    }
    $linenum++;
  }

  my @mulliken;
  for($i=0; $i<$nao; $i++) {
    @line = split(/ +/, $datafile[$start+4+$i]);
    $mulliken[$i] = $line[4];
  }
    
  if($start != 0) {
    return @mulliken;
  }
  
  printf "Error: Could not find Gross orbital populations in $_[0].\n";
  exit 1;
}

sub seek_mulliken_abp
{   
  my $noa = seek_natoms($_[0]);

  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close (OUT);

  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/-Atomic bond populations/) {
      $start = $linenum;
    }
    $linenum++;
  }

  my @mulliken;
  for($i=0; $i<$noa; $i++) {
    @line = split(/ +/, $datafile[$start+4+$i]);
    for($j=0; $j<$noa; $j++) {
      $mulliken[$noa*$i+$j] = $line[$j+2];
    }
  }

  if($start != 0) {
    return @mulliken;
  }
  
  printf "Error: Could not find Gross orbital populations in $_[0].\n";
  exit 1;
}

sub seek_mulliken_apnc
{   
  my $noa = seek_natoms($_[0]);

  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close (OUT);

  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/-Gross atomic populations/) {
      $start = $linenum;
    }
    $linenum++;
  }

  my @mulliken;
  for($i=0; $i<$noa; $i++) {
    @line = split(/ +/, $datafile[$start+4+$i]);
    for($j=0; $j<$noa; $j++) {
      $mulliken[$noa*$i+$j] = $line[$j+2];
    }
  }
  
  if($start != 0) {
    return @mulliken;
  }
  
  printf "Error: Could not find Gross atomic populations in $_[0].\n";
  exit 1;
}

sub seek_dipole
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close (OUT);

  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/-Electric dipole moment/) {
      $start = $linenum;
    }
    $linenum++;
  }

  my @dipole;
  for($i=0; $i<4; $i++) {
    @line = split(/ +/, $datafile[$start+2+$i]);
    $dipole[$i] = $line[3];
  }
  
  if($start != 0) {
    return @dipole;
  }
  
  printf "Error: Could not find Electronic dipole moment in $_[0].\n";
  exit 1;
}

sub seek_angmom
{   
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close (OUT);

  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/-Electronic angular momentum/) {
      $start = $linenum;
    }
    $linenum++;
  }

  my @angmom;
  for($i=0; $i<3; $i++) {
    @line = split(/ +/, $datafile[$start+2+$i]);
    $angmom[$i] = $line[3];
  }
  
  if($start != 0) {
    return @angmom;
  }
  
  printf "Error: Could not find Electronic angular momentum in $_[0].\n";
  exit 1;
}

sub seek_epef
{   
  my $noa = seek_natoms($_[0]);

  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close (OUT);

  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/-Electrostatic potential/) {
      $start = $linenum;
    }
    $linenum++;
  }

  my @epef;
  for($i=0; $i<$noa; $i++) {
    @line = split(/ +/, $datafile[$start+4+$i]);
    for($j=0; $j<4; $j++) {
      $epef[$noa*$i+$j] = $line[$j+2];
    }
  }
  
  if($start != 0) {
    return @epef;
  }
  
  printf "Error: Could not find Electrostatic potential\n";
  printf "       and electric field in $_[0].\n";
  exit 1;
}

sub seek_edensity
{   
  my $noa = seek_natoms($_[0]);

  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close (OUT);

  $linenum=0;
  $start = 0;
  foreach $line (@datafile) {
    if ($line =~ m/-Electron density/) {
      $start = $linenum;
    }
    $linenum++;
  }

  my @edensity;
  for($i=0; $i<$noa; $i++) {
    @line = split(/ +/, $datafile[$start+4+$i]);
    $edensity[$i] = $line[2];
  }
    
  if($start != 0) {
    return @edensity;
  }
  
  printf "Error: Could not find Electron density in $_[0].\n";
  exit 1;
}

sub seek_mvd
{   
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Total one-electron MVD terms/) {
      @data = split(/[ \t]+/, $_);
      $mvd = $data[6];
      return $mvd;
    }
  }
  close(OUT);

  printf "Error: Could not find Relativistic MVD one-electron\n";
  printf "       corrections in $_[0].\n";
  exit 1;
}

sub seek_pair_energies
{
  my $filename = $_[0];
  my $wfn = $_[1];
  my $spincase = $_[2];
  my @pair_energies;

  # This is the position of the desired pair energies
  my $pos;
  if ($wfn eq "MP2") {
    $pos = 3;
  }
  elsif ($wfn eq "CC") {
    $pos = 4;
  }
  else {
    return @pair_energies;
  }

  open(OUT, "$filename") || die "cannot open $filename $!";
  my @datafile = <OUT>;
  close(OUT);

  my $linenum=0;
  my $start = 0;
  my $end = 0;
  foreach $line (@datafile) {
    if ($line =~ m/$spincase pair energies/) {
      $start = $linenum+3;
    }
    if ($start != 0 && $start <= $linenum && $end == 0 && $line =~ m/-------------/) {
      $end = $linenum;
    }
    $linenum++;
  }

  for(my $i=$start; $i < $end; $i++) {
    @line = split(/ +/, $datafile[$i]);
    $pair_energies[$i-$start] = $line[$pos];
  }

  if($start != 0) {
    return @pair_energies;
  }

  printf "Error: Could not find $wfn $spincase pair energies in $filename.\n";
  exit 1;
}

sub seek_nstates
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  seek(OUT,0,0);
  while(<OUT>) {
    if (/Number of States =/) {
      @data = split(/[ \t]+/, $_);
      my $nstates = $data[5];
      return $nstates;
    }
  }
  close(OUT);

  printf "Error: Could not find the number of states in $_[0].\n";
  exit 1;
}

sub seek_excitation_energy
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $nstates = "$_[2]";
  $j=0;
  $linenum=0;
  foreach $line (@datafile) {
    $linenum++;
    if ($line =~ m/$match/) {
      while ($j<$nstates) {
        @test = split (/ +/,$datafile[$linenum+1+$j]);
        $ex[$j] = $test[5];
        $j++;
      }
    }
  }

  $OK = 1;
  for($i=0; $i < $nstates; $i++) {
    #printf "%d %8.4f\n", $i, $ex[$i];
    if($ex[$i] < 0.0) {
      $OK = 0; 
    }
  }
  
  if($OK && $nstates > 0) {
    return @ex;
  }

  printf "Error: Check $_[1] in $_[0].\n";
  exit 1;
}

sub seek_osc_str
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $nstates = "$_[2]";
  $j=0;
  $linenum=0;
  foreach $line (@datafile) {
    $linenum++;
    if ($line =~ m/$match/) {
      while ($j<$nstates) {
        @test = split (/ +/,$datafile[$linenum+1+$j]);
        $os[$j] = $test[6];
        $j++;
      }
    }
  }

  $OK = 1;
  for($i=0; $i < $nstates; $i++) {
    #printf "%d %8.4f\n", $i, $os[$i];
    if($os[$i] < 0.0) {
      $OK = 0; 
    }
  }
  
  if($OK && $nstates > 0) {
    return @os;
  }

  printf "Error: Check $_[1] in $_[0].\n";
  exit 1;
}

sub seek_rot_str
{
  open(OUT, "$_[0]") || die "cannot open $_[0] $!";
  @datafile = <OUT>;
  close(OUT);

  $match = "$_[1]";
  $nstates = "$_[2]";
  $j=0;
  $linenum=0;
  foreach $line (@datafile) {
    $linenum++;
    if ($line =~ m/$match/) {
      while ($j<$nstates) {
        @test = split (/ +/,$datafile[$linenum+1+$j]);
        $rs[$j] = $test[7];
        $j++;
      }
    }
  }

  if($nstates > 0) {
    return @rs;
  }

  printf "Error: Check $_[1] in $_[0].\n";
  exit 1;
}

sub compare_arrays
{
  my $A = $_[0];
  my $B = $_[1];
  my $dim = $_[2];
  my $tol = $_[3];
  my $OK = 1;
  my $i=0;
  
  for($i=0; $i < $dim; $i++) {
    if(abs(@$A[$i] - @$B[$i]) > $tol) {
      $OK = 0;
    }
  }

  return $OK;
}

sub build_psi_cmd
{
  $EXEC = $_[0];
  $QUIET = $_[1];
  $SRC_PATH = $_[2];
  $EXEC_PATH = $_[3];
  $EXTRA_ARGS = $_[4];

  $PSICMD = "";

  if($EXEC_PATH ne "") {
      $PSICMD = "PATH=$EXEC_PATH:\$PATH;export PATH;$EXEC";
  }
  else {
      $PSICMD = "$EXEC";
  }

  if($SRC_PATH ne "") {
      $PSICMD = $PSICMD . " -f $SRC_PATH/$PSITEST_INPUT";
  }

  if($QUIET == 1) {
      $PSICMD = $PSICMD . " 1>/dev/null 2>/dev/null";
  }

  if($EXTRA_ARGS ne "") {
      $PSICMD = $PSICMD . $EXTRA_ARGS;
  }

  return $PSICMD;
}

sub run_psi_command
{
  $test_name = get_test_name();
  my $target = "$test_name.$PSITEST_TARGET_SUFFIX";

  my $clean_only = 0;
  my $interrupted = 1;
  my $quiet = 0;
  my $ARGV;
  while ($ARGV = shift) {
    if   ("$ARGV" eq "-q") { $quiet = 1; }
    elsif("$ARGV" eq "-c") { $clean_only = 1; }
    elsif("$ARGV" eq "-u") { $interrupted = 0; }
    elsif("$ARGV" eq "-h") { usage_notice($PSITEST_TEST_SCRIPT); exit(1); }
  }

  my $exec = "";
  if($clean_only == 1) {
    $exec = "psiclean";
  }
  else {
    $exec = "psi3";
  }

  my $psicmd = build_psi_cmd($exec, $quiet, $SRC_PATH, $PSITEST_EXEC_PATH, "");

  my $psi_fail = system ("$psicmd");
  if ($clean_only == 1) {
    exit(0);
  }
  
  if ($psi_fail != 0) {
    open(RE, ">>$target") || die "cannot open $target $!"; 
    printf RE "Psi3 failed!\n";
    close (RE);
    printf STDOUT "Psi3 failed!\n";
    my $psicmd = build_psi_cmd("psiclean", 1, $SRC_PATH, $PSITEST_EXEC_PATH, "");
    system("$psicmd");
    exit($interrupted);
  }

  my @result = ($interrupted);
  return @result;
}

sub get_calctype_string
{
  # It's better to use File::Temp but it doesn't seem to be installed by default
  # use File::Temp;
  use POSIX qw(tmpnam);

  my $tempfile = tmpnam();
  my $psicmd = build_psi_cmd("psi3 -c", 0, $SRC_PATH,  $PSITEST_EXEC_PATH, " 1>$tempfile 2>/dev/null");
  my $psi_fail = system($psicmd);
  open(RE, "$tempfile") || die "cannot open $tempfile $!";
  my $calctype;
  my $wfn;
  my $jobtype;
  my $reftype;
  my $dertype;
  my $direct;
  seek(RE,0,0);
  while(<RE>) {
    if (/Calculation type string = /) {
      @data = split(/ +/, $_);
      $calctype = $data[4];
      $calctype =~ s/\n//;
    }
  }
  seek(RE,0,0);
  while(<RE>) {
    if (/Wavefunction            = /) {
      @data = split(/ +/, $_);
      $wfn = $data[2];
      $wfn =~ s/\n//;
    }
  }
  seek(RE,0,0);
  while(<RE>) {
    if (/Reference               = /) {
      @data = split(/ +/, $_);
      $reftype = $data[2];
      $reftype =~ s/\n//;
    }
  }
  seek(RE,0,0);
  while(<RE>) {
    if (/Jobtype                 = /) {
      @data = split(/ +/, $_);
      $jobtype = $data[2];
      $jobtype =~ s/\n//;
    }
  }
  seek(RE,0,0);
  while(<RE>) {
    if (/Dertype                 = /) {
      @data = split(/ +/, $_);
      $dertype = $data[2];
      $dertype =~ s/\n//;
    }
  }
  seek(RE,0,0);
  while(<RE>) {
    if (/Direct                  = /) {
      @data = split(/ +/, $_);
      my $tmp = $data[2];
      $tmp =~ s/\n//;
      if ($tmp eq "true") {
        $direct = 1;
      }
      elsif ($tmp eq "false") {
        $direct = 0;
      }
    }
  }
  close (RE);
  system("rm -f $tempfile");
  
  return ($calctype, $wfn, $reftype, $jobtype, $dertype, $direct);
}

1;

