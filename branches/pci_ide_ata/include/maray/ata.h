#ifndef __MARAY_ATA_H
#define __MARAY_ATA_H

#include <i386/types.h>

void ata_identify_device(void);
void ata_set_feature(void);
void ata_readdma_test(void);

#endif /* __MARAY_ATA_H */

