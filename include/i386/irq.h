#ifndef __I386_IRQ
#define __I386_IRQ

#define IRQ_OFFSET	0x20		/* int number of the first irq */

#define TIMER_IRQ		0x20	/*IRQ0 */
#define	KB_IRQ			0x21	/*IRQ1 */	
#define	X2_IRQ			0x22	/*IRQ2 */
#define	COM2_IRQ		0x23	/*IRQ3 */
#define	COM1_IRQ		0x24	/*IRQ4 */
#define	LPT2_IRQ		0x25	/*IRQ5 */
#define	FDD_IRQ			0x26	/*IRQ6  */
#define	LPT1_IRQ		0x27 	/*IRQ7  */

#define	CMOSRT_IRQ	0x28 	/*IRQ8 */
#define	X9_IRQ			0x29 	/*IRQ9 */
#define	X10_IRQ			0x2A 	/*IRQ10*/
#define	X11_IRQ			0x2B 	/*IRQ11*/
#define	MOUSE_IRQ		0x2C 	/*IRQ12*/
#define	NUM_COP_IRQ	0x2D 	/*IRQ13*/
#define	HDD_IRQ			0x2E 	/*IRQ14*/
#define	X15_IRQ			0x2F	/*IRQ15*/



void init_irq();
void enable_irq(int irq);
void disable_irq(int irq);

#endif
