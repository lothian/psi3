#include <stdio.h>
#include <dpd.h>
#define EXTERN
#include "globals.h"

void GaeL2(void)
{
  dpdbuf4 newLijab, newLIJAB, newLIjAb;
  dpdbuf4 D;
  dpdfile2 GAE, Gae;

  /* RHS += P(ab)<ij||ae>Gbe */
  dpd_file2_init(&GAE, CC_OEI, 0, 1, 1, "GAE");
  dpd_file2_init(&Gae, CC_OEI, 0, 1, 1, "Gae");

  dpd_buf4_init(&newLIJAB, CC_LAMPS, 0, 2, 5, 2, 7, 0, "New LIJAB");

  dpd_buf4_init(&D, CC_DINTS, 0, 2, 5, 2, 5, 0, "D <ij||ab> (i>j,ab)");
  dpd_contract424(&D, &GAE, &newLIJAB, 3, 1, 0, 1.0, 1.0);
  dpd_contract244(&GAE, &D, &newLIJAB, 1, 2, 1, 1.0, 1.0);

  dpd_buf4_close(&newLIJAB);


  dpd_buf4_init(&newLijab, CC_LAMPS, 0, 2, 5, 2, 7, 0, "New Lijab");

  dpd_contract424(&D, &Gae, &newLijab, 3, 1, 0, 1.0, 1.0);
  dpd_contract244(&Gae, &D, &newLijab, 1, 2, 1, 1.0, 1.0);
  dpd_buf4_close(&D);

  dpd_buf4_close(&newLijab);


  dpd_buf4_init(&newLIjAb, CC_LAMPS, 0, 0, 5, 0, 5, 0, "New LIjAb");

  dpd_buf4_init(&D, CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  dpd_contract424(&D, &Gae, &newLIjAb, 3, 1, 0, 1.0, 1.0);
  dpd_contract244(&GAE, &D, &newLIjAb, 1, 2, 1, 1.0, 1.0);
  dpd_buf4_close(&D);

  dpd_buf4_close(&newLIjAb);

  dpd_file2_close(&GAE);
  dpd_file2_close(&Gae);

}

void GmiL2(void)
{

  dpdbuf4 newLijab, newLIJAB, newLIjAb;
  dpdbuf4 D;
  dpdfile2 GMI, Gmi;

  /* RHS -= P(ij) * <im||ab> * Gmj */

  dpd_file2_init(&GMI, CC_OEI, 0, 0, 0, "GMI");
  dpd_file2_init(&Gmi, CC_OEI, 0, 0, 0, "Gmi");

  dpd_buf4_init(&newLIJAB, CC_LAMPS, 0, 0, 7, 2, 7, 0, "New LIJAB");

  dpd_buf4_init(&D, CC_DINTS, 0, 0, 7, 0, 7, 0, "D <ij||ab> (ij,a>b)");
  dpd_contract424(&D, &GMI, &newLIJAB, 1, 0, 1, -1.0, 1.0);
  dpd_contract244(&GMI, &D, &newLIJAB, 0, 0, 0, -1.0, 1.0);

  dpd_buf4_close(&newLIJAB);


  dpd_buf4_init(&newLijab, CC_LAMPS, 0, 0, 7, 2, 7, 0, "New Lijab");

  dpd_contract424(&D, &Gmi, &newLijab, 1, 0, 1, -1.0, 1.0);
  dpd_contract244(&Gmi, &D, &newLijab, 0, 0, 0, -1.0, 1.0);
  dpd_buf4_close(&D);

  dpd_buf4_close(&newLijab);


  dpd_buf4_init(&newLIjAb, CC_LAMPS, 0, 0, 5, 0, 5, 0, "New LIjAb");

  dpd_buf4_init(&D, CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  dpd_contract424(&D, &Gmi, &newLIjAb, 1, 0, 1, -1.0, 1.0);
  dpd_contract244(&GMI, &D, &newLIjAb, 0, 0, 0, -1.0, 1.0);
  dpd_buf4_close(&D);

  dpd_buf4_close(&newLIjAb);

  dpd_file2_close(&Gmi);
  dpd_file2_close(&GMI);

}
