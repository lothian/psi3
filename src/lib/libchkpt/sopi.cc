/*!
  \file sopi.c
  \ingroup (CHKPT)
*/

#include <stdio.h>
#include <stdlib.h>
#include <psifiles.h>
#include <libpsio/psio.hpp>
extern "C" {
	#include <libchkpt/chkpt.h>
}
#include <libchkpt/chkpt.hpp>

using namespace psi;

int *Chkpt::rd_sopi(void)
{
	int nirreps, *sopi;
	char *keyword;
	keyword = build_keyword("SO's per irrep");

	nirreps = rd_nirreps();
	sopi = array<int>(nirreps);

	psio->read_entry(PSIF_CHKPT, keyword, (char *) sopi, nirreps*sizeof(int));

	free(keyword);
	return sopi;
}

void Chkpt::wt_sopi(int *sopi)
{
	int nirreps;
	char *keyword;
	keyword = build_keyword("SO's per irrep");

	nirreps = rd_nirreps();

	psio->write_entry(PSIF_CHKPT, keyword, (char *) sopi, nirreps*sizeof(int));

	free(keyword);
}

extern "C" {
/*!
** chkpt_rd_sopi()
** Reads in the number of symmetry orbitals in each irrep.
**
**  takes no arguments.
**
**  returns:
**    sopi =  an array which has an element for each irrep of the
**            point group of the molecule (n.b. not just the ones
**            with a non-zero number of basis functions). each 
**            element contains the number of symmetry orbitals for
**            that irrep. Also, see chkpt_rd_orbspi().
**
** \ingroup (CHKPT)
*/
	int *chkpt_rd_sopi(void)
	{
		return _default_chkpt_lib_->rd_sopi();
	}

/*!
** chkpt_wt_sopi():  Writes out the number of symmetry orbitals in each irrep.
**
** \param sopi = an array which has an element for each irrep of the
**               point group of the molecule (n.b. not just the ones
**               with a non-zero number of basis functions). each 
**               element contains the number of symmetry orbitals for
**               that irrep. Also, see chkpt_rd_orbspi().
**
** returns: none
**
** \ingroup (CHKPT)
*/
	void chkpt_wt_sopi(int *sopi)
	{
		_default_chkpt_lib_->wt_sopi(sopi);
	}
}
