#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

MODULE_AUTHOR("Stepan Belyakov");
MODULE_LICENSE( "GPL v2" );

#define SHARED_IRQ 17
#define TIME_INTERVAL 60000

static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0, irq_counter_prev = 0;
module_param(irq, in, S_IRUGO);

struct timer_list my_timer;


static irqreturn_t my_interrupt(int irq, void *dev_id) {
    irq_counter++;
    printk( KERN_INFO "In the ISR: counter = %d\n", irq_counter );
    return IRQ_NONE; 
}

static void logger_func(struct timer_list* tlist) {
    printk(KERN_INFO "clicks amount: %d\n", irq_counter - irq_counter_prev);
    irq_counter_prev = irq_counter;
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIME_INTERVAL));
}

static int __init my_init( void ) {
    if (request_irq(irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id))
        return -1;
    printk(KERN_INFO "Successfully loading ISR handler on IRQ %d\n", irq);
    
    timer_setup(&my_timer, logger_func, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIME_INTERVAL));
    
    return 0;
}

static void __exit my_exit( void ) {
    synchronize_irq(irq);
    free_irq(irq, &my_dev_id);
    //del_timer(&my_timer);
    printk(KERN_INFO "Successfully unloading, irq_counter = %d\n", irq_counter);
}


module_init(my_init);
module_exit(my_exit);

