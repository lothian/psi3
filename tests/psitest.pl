#!/usr/bin/perl 

# Some general functions useful for parsing PSI3 output files.
# TDC, 5/03

sub pass_test
{
  printf "%-24s...PASSED\n", $_[0];
}

sub fail_test
{
  printf "%-24s...FAILED\n", $_[0];
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

  foreach $line (@datafile) {
    if($line =~ m/$match/) {
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

  if($lastiter != 0) {
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

  foreach $line (@datafile) {
    if($line =~ m/$match/) {
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

  if($lastiter != 0) {
    return @grad;
  }

  printf "Error: Could not find $_[1] grad in $_[0].\n";
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
    if ($line =~ m/Largest components of/) {
      @test = split (/ +/,$datafile[$linenum-3]);
      $evals[$eval] = $test[5];
      $eval++;
    }
  }

  if($eval != 0) {
    return @evals;
  }

  printf "Error: Could not find EOM-CCSD energies in $_[0].\n";
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

sub compare_arrays
{
  $A = $_[0];
  $B = $_[1];
  $row = $_[2];
  $col = $_[3];
  $tol = $_[4];

  $OK = 1;
  for($i=0; $i < $row*$col; $i++) {
#    printf "%d %20.12f %20.12f\n", $i, @$A[$i], @$B[$i];
    if(abs(@$A[$i] - @$B[$i]) > $tol) {
      $OK = 0;
    }
  }

  return $OK;
}

1;

