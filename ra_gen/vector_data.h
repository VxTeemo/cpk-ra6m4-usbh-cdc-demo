/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (9)
        #endif
        /* ISR prototypes */
        void sci_uart_rxi_isr(void);
        void sci_uart_txi_isr(void);
        void sci_uart_tei_isr(void);
        void sci_uart_eri_isr(void);
        void r_icu_isr(void);
        void usbfs_interrupt_handler(void);
        void usbfs_resume_handler(void);
        void usbfs_d0fifo_handler(void);
        void usbfs_d1fifo_handler(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_SCI7_RXI ((IRQn_Type) 0) /* SCI7 RXI (Received data full) */
        #define SCI7_RXI_IRQn          ((IRQn_Type) 0) /* SCI7 RXI (Received data full) */
        #define VECTOR_NUMBER_SCI7_TXI ((IRQn_Type) 1) /* SCI7 TXI (Transmit data empty) */
        #define SCI7_TXI_IRQn          ((IRQn_Type) 1) /* SCI7 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI7_TEI ((IRQn_Type) 2) /* SCI7 TEI (Transmit end) */
        #define SCI7_TEI_IRQn          ((IRQn_Type) 2) /* SCI7 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI7_ERI ((IRQn_Type) 3) /* SCI7 ERI (Receive error) */
        #define SCI7_ERI_IRQn          ((IRQn_Type) 3) /* SCI7 ERI (Receive error) */
        #define VECTOR_NUMBER_ICU_IRQ0 ((IRQn_Type) 4) /* ICU IRQ0 (External pin interrupt 0) */
        #define ICU_IRQ0_IRQn          ((IRQn_Type) 4) /* ICU IRQ0 (External pin interrupt 0) */
        #define VECTOR_NUMBER_USBFS_INT ((IRQn_Type) 5) /* USBFS INT (USBFS interrupt) */
        #define USBFS_INT_IRQn          ((IRQn_Type) 5) /* USBFS INT (USBFS interrupt) */
        #define VECTOR_NUMBER_USBFS_RESUME ((IRQn_Type) 6) /* USBFS RESUME (USBFS resume interrupt) */
        #define USBFS_RESUME_IRQn          ((IRQn_Type) 6) /* USBFS RESUME (USBFS resume interrupt) */
        #define VECTOR_NUMBER_USBFS_FIFO_0 ((IRQn_Type) 7) /* USBFS FIFO 0 (DMA transfer request 0) */
        #define USBFS_FIFO_0_IRQn          ((IRQn_Type) 7) /* USBFS FIFO 0 (DMA transfer request 0) */
        #define VECTOR_NUMBER_USBFS_FIFO_1 ((IRQn_Type) 8) /* USBFS FIFO 1 (DMA transfer request 1) */
        #define USBFS_FIFO_1_IRQn          ((IRQn_Type) 8) /* USBFS FIFO 1 (DMA transfer request 1) */
        #endif /* VECTOR_DATA_H */