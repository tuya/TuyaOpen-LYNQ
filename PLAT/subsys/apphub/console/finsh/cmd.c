
#include <rtthread.h>
#include <string.h>
#ifdef RT_USING_FINSH
#include <finsh.h>


extern rt_tick_t rt_tick_get(void);
extern int skip_atoi(const char **s);


#define LIST_FIND_OBJ_NR 8
#if defined(__GNUC__)
rt_base_t rt_hw_interrupt_disable(void)
{
    register rt_uint16_t temp = 0;
    // asm("STC  FLG, %0":"=r" (temp));
    // asm("FCLR I");
    return (rt_base_t)temp;
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    // register rt_uint16_t temp;
    // temp = level & 0xffff;
    // asm("LDC %0, FLG": :"r" (temp));
}
#endif

static long clear(void)
{
    rt_kprintf("\x1b[2J\x1b[H");

    return 0;
}
MSH_CMD_EXPORT(clear, clear the terminal screen);

void rt_show_version(void){
    rt_kprintf("EC718 FinSH v1.0\n\r");
}
long version(void)
{
    rt_show_version();
    return 0;
}
MSH_CMD_EXPORT(version, show version information);

rt_inline void object_split(int len)
{
    while (len--) rt_kprintf("-");
}
enum rt_object_info_type
{
    RT_Object_Info_Thread = 0,                         /**< The object is a thread. */
#ifdef RT_USING_SEMAPHORE
    RT_Object_Info_Semaphore,                          /**< The object is a semaphore. */
#endif
#ifdef RT_USING_MUTEX
    RT_Object_Info_Mutex,                              /**< The object is a mutex. */
#endif
#ifdef RT_USING_EVENT
    RT_Object_Info_Event,                              /**< The object is a event. */
#endif
#ifdef RT_USING_MAILBOX
    RT_Object_Info_MailBox,                            /**< The object is a mail box. */
#endif
#ifdef RT_USING_MESSAGEQUEUE
    RT_Object_Info_MessageQueue,                       /**< The object is a message queue. */
#endif
#ifdef RT_USING_MEMHEAP
    RT_Object_Info_MemHeap,                            /**< The object is a memory heap */
#endif
#ifdef RT_USING_MEMPOOL
    RT_Object_Info_MemPool,                            /**< The object is a memory pool. */
#endif
#ifdef RT_USING_DEVICE
    RT_Object_Info_Device,                             /**< The object is a device */
#endif
    RT_Object_Info_Timer,                              /**< The object is a timer. */
#ifdef RT_USING_MODULE
    RT_Object_Info_Module,                             /**< The object is a module. */
#endif
#ifdef RT_USING_HEAP
    RT_Object_Info_Memory,                            /**< The object is a memory. */
#endif
#ifdef RT_USING_SMART
    RT_Object_Info_Channel,                            /**< The object is a IPC channel */
#endif
#ifdef RT_USING_HEAP
    RT_Object_Info_Custom,                             /**< The object is a custom object */
#endif
    RT_Object_Info_Unknown,                            /**< The object is unknown. */
};
#define _OBJ_CONTAINER_LIST_INIT(c)     \
    {&(_object_container[c].object_list), &(_object_container[c].object_list)}

static struct rt_object_information _object_container[RT_Object_Info_Unknown] =
{
    /* initialize object container - thread */
    {RT_Object_Class_Thread, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Thread), sizeof(struct rt_thread)},
#ifdef RT_USING_SEMAPHORE
    /* initialize object container - semaphore */
    {RT_Object_Class_Semaphore, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Semaphore), sizeof(struct rt_semaphore)},
#endif
#ifdef RT_USING_MUTEX
    /* initialize object container - mutex */
    {RT_Object_Class_Mutex, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Mutex), sizeof(struct rt_mutex)},
#endif
#ifdef RT_USING_EVENT
    /* initialize object container - event */
    {RT_Object_Class_Event, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Event), sizeof(struct rt_event)},
#endif
#ifdef RT_USING_MAILBOX
    /* initialize object container - mailbox */
    {RT_Object_Class_MailBox, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_MailBox), sizeof(struct rt_mailbox)},
#endif
#ifdef RT_USING_MESSAGEQUEUE
    /* initialize object container - message queue */
    {RT_Object_Class_MessageQueue, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_MessageQueue), sizeof(struct rt_messagequeue)},
#endif
#ifdef RT_USING_MEMHEAP
    /* initialize object container - memory heap */
    {RT_Object_Class_MemHeap, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_MemHeap), sizeof(struct rt_memheap)},
#endif
#ifdef RT_USING_MEMPOOL
    /* initialize object container - memory pool */
    {RT_Object_Class_MemPool, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_MemPool), sizeof(struct rt_mempool)},
#endif
#ifdef RT_USING_DEVICE
    /* initialize object container - device */
    {RT_Object_Class_Device, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Device), sizeof(struct rt_device)},
#endif
    /* initialize object container - timer */
    {RT_Object_Class_Timer, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Timer), sizeof(struct rt_timer)},
#ifdef RT_USING_MODULE
    /* initialize object container - module */
    {RT_Object_Class_Module, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Module), sizeof(struct rt_dlmodule)},
#endif
#ifdef RT_USING_HEAP
    /* initialize object container - small memory */
    {RT_Object_Class_Memory, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Memory), sizeof(struct rt_memory)},
#endif
#ifdef RT_USING_SMART
    /* initialize object container - module */
    {RT_Object_Class_Channel, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Channel), sizeof(struct rt_channel)},
    {RT_Object_Class_Custom, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Custom), sizeof(struct rt_custom_object)},
#endif
};

typedef struct
{
    rt_list_t *list;
    rt_list_t **array;
    rt_uint8_t type;
    int nr;             /* input: max nr, can't be 0 */
    int nr_out;         /* out: got nr */
} list_get_next_t;
struct rt_object_information *
rt_object_get_information(enum rt_object_class_type type)
{
    int index;

    for (index = 0; index < RT_Object_Info_Unknown; index ++)
        if (_object_container[index].type == type) return &_object_container[index];

    return RT_NULL;
}
// RTM_EXPORT(rt_object_get_information);
static void list_find_init(list_get_next_t *p, rt_uint8_t type, rt_list_t **array, int nr)
{
    struct rt_object_information *info;
    rt_list_t *list;

    info = rt_object_get_information((enum rt_object_class_type)type);
    list = &info->object_list;

    p->list = list;
    p->type = type;
    p->array = array;
    p->nr = nr;
    p->nr_out = 0;
}

static rt_list_t *list_get_next(rt_list_t *current, list_get_next_t *arg)
{
    int first_flag = 0;
    rt_base_t level;
    rt_list_t *node, *list;
    rt_list_t **array;
    int nr = 0;  // 初始化nr变量

    arg->nr_out = 0;
    
    // 初始化结构体成员以避免未初始化警告
    if (arg != RT_NULL) {
        if (arg->array == RT_NULL) {
            arg->array = (rt_list_t **)rt_malloc(sizeof(rt_list_t *) * arg->nr);
        }
        
        if (arg->list == RT_NULL) {
            return (rt_list_t *)RT_NULL;
        }
    }

    list = arg->list;

    if (!current) /* find first */
    {
        node = list;
        first_flag = 1;
    }
    else
    {
        node = current;
    }

    level = rt_hw_interrupt_disable();

    if (!first_flag)
    {
        struct rt_object *obj;
        /* The node in the list? */
        obj = rt_list_entry(node, struct rt_object, list);
        if ((obj->type & ~RT_Object_Class_Static) != arg->type)
        {
            rt_hw_interrupt_enable(level);
            return (rt_list_t *)RT_NULL;
        }
    }

    nr = 0;
    array = arg->array;
    while (1)
    {
        node = node->next;

        if (node == list)
        {
            node = (rt_list_t *)RT_NULL;
            break;
        }
        nr++;
        *array++ = node;
        if (nr == arg->nr)
        {
            break;
        }
    }

    rt_hw_interrupt_enable(level);
    arg->nr_out = nr;
    return node;
}
static const char* rt_errno_strs[] =
{
    "OK",
    "ERROR",
    "ETIMOUT",
    "ERSFULL",
    "ERSEPTY",
    "ENOMEM",
    "ENOSYS",
    "EBUSY",
    "EIO",
    "EINTRPT",
    "EINVAL",
    "EUNKNOW"
};

const char *rt_strerror(rt_err_t error)
{
    if (error < 0)
        error = -error;

    return (error > RT_EINVAL + 1) ?
           rt_errno_strs[RT_EINVAL + 1] :
           rt_errno_strs[error];
}
// RTM_EXPORT(rt_strerror);

long list_thread(void)
{
    rt_base_t level;
    list_get_next_t find_arg = {0};
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;
    const char *item_title = "thread";
    int maxlen;
    // list_find_init(&find_arg, RT_Object_Class_Thread, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));
    maxlen = RT_NAME_MAX;
#ifdef RT_USING_SMP
    rt_kprintf("%-*.*s cpu bind pri  status      sp     stack size max used left tick  error\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" --- ---- ---  ------- ---------- ----------  ------  ---------- ---\n\r");
#else
    rt_kprintf("%-*.*s pri  status      sp     stack size max used left tick  error\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ---  ------- ---------- ----------  ------  ---------- ---\n\r");
#endif /*RT_USING_SMP*/

    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_thread thread_info, *thread;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();

                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }
                /* copy info */
                memcpy(&thread_info, obj, sizeof thread_info);
                rt_hw_interrupt_enable(level);

                thread = (struct rt_thread *)obj;
                {
                    rt_uint8_t stat;
                    rt_uint8_t *ptr;

#ifdef RT_USING_SMP
                    if (thread->oncpu != RT_CPU_DETACHED)
                        rt_kprintf("%-*.*s %3d %3d %4d ", maxlen, RT_NAME_MAX, thread->name, thread->oncpu, thread->bind_cpu, thread->current_priority);
                    else
                        rt_kprintf("%-*.*s N/A %3d %4d ", maxlen, RT_NAME_MAX, thread->name, thread->bind_cpu, thread->current_priority);

#else
                    rt_kprintf("%-*.*s %3d ", maxlen, RT_NAME_MAX, thread->name, thread->current_priority);
#endif /*RT_USING_SMP*/
                    stat = (thread->stat & RT_THREAD_STAT_MASK);
                    if (stat == RT_THREAD_READY)        rt_kprintf(" ready  ");
                    else if ((stat & RT_THREAD_SUSPEND_MASK) == RT_THREAD_SUSPEND_MASK) rt_kprintf(" suspend");
                    else if (stat == RT_THREAD_INIT)    rt_kprintf(" init   ");
                    else if (stat == RT_THREAD_CLOSE)   rt_kprintf(" close  ");
                    else if (stat == RT_THREAD_RUNNING) rt_kprintf(" running");

#if defined(ARCH_CPU_STACK_GROWS_UPWARD)
                    ptr = (rt_uint8_t *)thread->stack_addr + thread->stack_size - 1;
                    while (*ptr == '#')ptr --;

                    rt_kprintf(" 0x%08x 0x%08x    %02d%%   0x%08x %03d\n\r",
                               ((rt_ubase_t)thread->sp - (rt_ubase_t)thread->stack_addr),
                               thread->stack_size,
                               ((rt_ubase_t)ptr - (rt_ubase_t)thread->stack_addr) * 100 / thread->stack_size,
                               thread->remaining_tick,
                               thread->error);
#else
                    ptr = (rt_uint8_t *)thread->stack_addr;
                    while (*ptr == '#') ptr ++;
                    rt_kprintf(" 0x%08x 0x%08x    %02d%%   0x%08x %s\n\r",
                               thread->stack_size + ((rt_ubase_t)thread->stack_addr - (rt_ubase_t)thread->sp),
                               thread->stack_size,
                               (thread->stack_size - ((rt_ubase_t) ptr - (rt_ubase_t) thread->stack_addr)) * 100
                               / thread->stack_size,
                               thread->remaining_tick,
                               "OK"); //rt_strerror(thread->error)
#endif
                }
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}

static void show_wait_queue(struct rt_list_node *list)
{
    struct rt_thread *thread;
    struct rt_list_node *node;

    for (node = list->next; node != list; node = node->next)
    {
        thread = rt_list_entry(node, struct rt_thread, tlist);
        rt_kprintf("%.*s", RT_NAME_MAX, thread->name);

        if (node->next != list)
            rt_kprintf("/");
    }
}

#ifdef RT_USING_SEMAPHORE
long list_sem(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    int maxlen;
    const char *item_title = "semaphore";

    list_find_init(&find_arg, RT_Object_Class_Semaphore, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    maxlen = RT_NAME_MAX;

    rt_kprintf("%-*.*s v   suspend thread\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" --- --------------\n\r");

    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_semaphore *sem;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }
                rt_hw_interrupt_enable(level);

                sem = (struct rt_semaphore *)obj;
                if (!rt_list_isempty(&sem->parent.suspend_thread))
                {
                    rt_kprintf("%-*.*s %03d %d:",
                               maxlen, RT_NAME_MAX,
                               sem->parent.parent.name,
                               sem->value,
                               rt_list_len(&sem->parent.suspend_thread));
                    show_wait_queue(&(sem->parent.suspend_thread));
                    rt_kprintf("\n\r");
                }
                else
                {
                    rt_kprintf("%-*.*s %03d %d\n\r",
                               maxlen, RT_NAME_MAX,
                               sem->parent.parent.name,
                               sem->value,
                               rt_list_len(&sem->parent.suspend_thread));
                }
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}
#endif /* RT_USING_SEMAPHORE */

#ifdef RT_USING_EVENT
long list_event(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;
    int maxlen;
    const char *item_title = "event";
    list_find_init(&find_arg, RT_Object_Class_Event, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));
    maxlen = RT_NAME_MAX;
    rt_kprintf("%-*.*s      set    suspend thread\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf("  ---------- --------------\n\r");
    do{
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_event *e;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                e = (struct rt_event *)obj;
                if (!rt_list_isempty(&e->parent.suspend_thread))
                {
                    rt_kprintf("%-*.*s  0x%08x %03d:",
                               maxlen, RT_NAME_MAX,
                               e->parent.parent.name,
                               e->set,
                               rt_list_len(&e->parent.suspend_thread));
                    show_wait_queue(&(e->parent.suspend_thread));
                    rt_kprintf("\n\r");
                }
                else{
                    rt_kprintf("%-*.*s  0x%08x 0\n\r",
                            maxlen, RT_NAME_MAX, e->parent.parent.name, e->set);
                }
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}
#endif /* RT_USING_EVENT */

#ifdef RT_USING_MUTEX
long list_mutex(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    int maxlen;
    const char *item_title = "mutex";

    list_find_init(&find_arg, RT_Object_Class_Mutex, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    maxlen = RT_NAME_MAX;

    rt_kprintf("%-*.*s   owner  hold priority suspend thread \n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" -------- ---- -------- --------------\n\r");

    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_mutex *m;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                m = (struct rt_mutex *)obj;
                if (!rt_list_isempty(&m->parent.suspend_thread))
                {
                    rt_kprintf("%-*.*s %-8.*s %04d %8d  %04d ",
                           maxlen, RT_NAME_MAX,
                           m->parent.parent.name,
                           RT_NAME_MAX,
                           m->owner->name,
                           m->hold,
                           m->priority,
                           rt_list_len(&m->parent.suspend_thread));
                    show_wait_queue(&(m->parent.suspend_thread));
                    rt_kprintf("\n\r");
                }
                else
                {
                    rt_kprintf("%-*.*s %-8.*s %04d %8d  %04d\n\r",
                           maxlen, RT_NAME_MAX,
                           m->parent.parent.name,
                           RT_NAME_MAX,
                           m->owner->name,
                           m->hold,
                           m->priority,
                           rt_list_len(&m->parent.suspend_thread));
                }
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}
#endif /* RT_USING_MUTEX */

#ifdef RT_USING_MAILBOX
long list_mailbox(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    int maxlen;
    const char *item_title = "mailbox";
    list_find_init(&find_arg, RT_Object_Class_MailBox, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));
    maxlen = RT_NAME_MAX;
    rt_kprintf("%-*.*s entry size suspend thread\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ----  ---- --------------\n\r");
    do{
        next = list_get_next(next, &find_arg);{
            int i;
            for (i = 0; i < find_arg.nr_out; i++){
                struct rt_object *obj;
                struct rt_mailbox *m;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                m = (struct rt_mailbox *)obj;
                if (!rt_list_isempty(&m->parent.suspend_thread))
                {
                    rt_kprintf("%-*.*s %04d  %04d %d:",
                               maxlen, RT_NAME_MAX,
                               m->parent.parent.name,
                               m->entry,
                               m->size,
                               rt_list_len(&m->parent.suspend_thread));
                    show_wait_queue(&(m->parent.suspend_thread));
                    rt_kprintf("\n\r");
                }
                else{
                    rt_kprintf("%-*.*s %04d  %04d %d\n\r",
                               maxlen, RT_NAME_MAX,
                               m->parent.parent.name,
                               m->entry,
                               m->size,
                               rt_list_len(&m->parent.suspend_thread));
                }
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);
    return 0;
}
#endif /* RT_USING_MAILBOX */

#ifdef RT_USING_MESSAGEQUEUE
long list_msgqueue(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    int maxlen;
    const char *item_title = "msgqueue";

    list_find_init(&find_arg, RT_Object_Class_MessageQueue, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    maxlen = RT_NAME_MAX;

    rt_kprintf("%-*.*s entry suspend thread\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ------------------\n\r");
    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_messagequeue *m;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                m = (struct rt_messagequeue *)obj;
                if (!rt_list_isempty(&m->parent.suspend_thread))
                {
                    rt_kprintf("%-*.*s %04d  %d:",
                               maxlen, RT_NAME_MAX,
                               m->parent.parent.name,
                               m->entry,
                               rt_list_len(&m->parent.suspend_thread));
                    show_wait_queue(&(m->parent.suspend_thread));
                    rt_kprintf("\n\r");
                }
                else
                {
                    rt_kprintf("%-*.*s %04d  %d\n\r",
                               maxlen, RT_NAME_MAX,
                               m->parent.parent.name,
                               m->entry,
                               rt_list_len(&m->parent.suspend_thread));
                }
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}
#endif /* RT_USING_MESSAGEQUEUE */

#ifdef RT_USING_MEMHEAP
long list_memheap(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    int maxlen;
    const char *item_title = "memheap";

    list_find_init(&find_arg, RT_Object_Class_MemHeap, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    maxlen = RT_NAME_MAX;

    rt_kprintf("%-*.*s  pool size  max used size available size\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ---------- ------------- --------------\n\r");
    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_memheap *mh;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                mh = (struct rt_memheap *)obj;

                rt_kprintf("%-*.*s %-010d %-013d %-05d\n\r",
                           maxlen, RT_NAME_MAX,
                           mh->parent.name,
                           mh->pool_size,
                           mh->max_used_size,
                           mh->available_size);

            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}
#endif /* RT_USING_MEMHEAP */

#ifdef RT_USING_MEMPOOL
long list_mempool(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    int maxlen;
    const char *item_title = "mempool";

    list_find_init(&find_arg, RT_Object_Class_MemPool, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    maxlen = RT_NAME_MAX;

    rt_kprintf("%-*.*s block total free suspend thread\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ----  ----  ---- --------------\n\r");
    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_mempool *mp;
                int suspend_thread_count;
                rt_list_t *node;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                mp = (struct rt_mempool *)obj;

                suspend_thread_count = 0;
                rt_list_for_each(node, &mp->suspend_thread)
                {
                    suspend_thread_count++;
                }

                if (suspend_thread_count > 0)
                {
                    rt_kprintf("%-*.*s %04d  %04d  %04d %d:",
                               maxlen, RT_NAME_MAX,
                               mp->parent.name,
                               mp->block_size,
                               mp->block_total_count,
                               mp->block_free_count,
                               suspend_thread_count);
                    show_wait_queue(&(mp->suspend_thread));
                    rt_kprintf("\n\r");
                }
                else
                {
                    rt_kprintf("%-*.*s %04d  %04d  %04d %d\n\r",
                               maxlen, RT_NAME_MAX,
                               mp->parent.name,
                               mp->block_size,
                               mp->block_total_count,
                               mp->block_free_count,
                               suspend_thread_count);
                }
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}
#endif /* RT_USING_MEMPOOL */

long list_timer(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    int maxlen;
    const char *item_title = "timer";

    list_find_init(&find_arg, RT_Object_Class_Timer, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    maxlen = RT_NAME_MAX;

    rt_kprintf("%-*.*s  periodic   timeout    activated     mode\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" ---------- ---------- ----------- ---------\n\r");
    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_timer *timer;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                timer = (struct rt_timer *)obj;
                rt_kprintf("%-*.*s 0x%08x 0x%08x ",
                           maxlen, RT_NAME_MAX,
                           timer->parent.name,
                           timer->init_tick,
                           timer->timeout_tick);
                if (timer->parent.flag & RT_TIMER_FLAG_ACTIVATED)
                    rt_kprintf("activated   ");
                else
                    rt_kprintf("deactivated ");
                if (timer->parent.flag & RT_TIMER_FLAG_PERIODIC)
                    rt_kprintf("periodic\n\r");
                else
                    rt_kprintf("one shot\n\r");

            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    rt_kprintf("current tick:0x%08x\n\r", rt_tick_get());

    return 0;
}

#ifdef RT_USING_DEVICE
static char *const device_type_str[RT_Device_Class_Unknown] =
{
    "Character Device",
    "Block Device",
    "Network Interface",
    "MTD Device",
    "CAN Device",
    "RTC",
    "Sound Device",
    "Graphic Device",
    "I2C Bus",
    "USB Slave Device",
    "USB Host Bus",
    "USB OTG Bus",
    "SPI Bus",
    "SPI Device",
    "SDIO Bus",
    "PM Pseudo Device",
    "Pipe",
    "Portal Device",
    "Timer Device",
    "Miscellaneous Device",
    "Sensor Device",
    "Touch Device",
    "Phy Device",
    "Security Device",
    "WLAN Device",
    "Pin Device",
    "ADC Device",
    "DAC Device",
    "WDT Device",
    "PWM Device",
};

long list_device(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;
    const char *device_type;

    int maxlen;
    const char *item_title = "device";

    list_find_init(&find_arg, RT_Object_Class_Device, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    maxlen = RT_NAME_MAX;

    rt_kprintf("%-*.*s         type         ref count\n\r", maxlen, maxlen, item_title);
    object_split(maxlen);
    rt_kprintf(" -------------------- ----------\n\r");
    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_device *device;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                device = (struct rt_device *)obj;
                device_type = "Unknown";
                if (device->type < RT_Device_Class_Unknown &&
                    device_type_str[device->type] != RT_NULL)
                {
                    device_type = device_type_str[device->type];
                }
                rt_kprintf("%-*.*s %-20s %-8d\n\r",
                           maxlen, RT_NAME_MAX,
                           device->parent.name,
                           device_type,
                           device->ref_count);

            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}
#endif /* RT_USING_DEVICE */

int cmd_list(int argc, char **argv)
{
    if(argc > 2)
    {
        rt_kprintf("%d:%s %d\n\r",argc,argv[1],skip_atoi((const char **)&argv[2]));
        return 0;
    }
    else if(argc == 2)
    {
        if(strcmp(argv[1], "thread") == 0)
        {
            list_thread();
        }
        else if(strcmp(argv[1], "timer") == 0)
        {
            list_timer();
        }
#ifdef RT_USING_SEMAPHORE
        else if(strcmp(argv[1], "sem") == 0)
        {
            list_sem();
        }
#endif /* RT_USING_SEMAPHORE */
#ifdef RT_USING_EVENT
        else if(strcmp(argv[1], "event") == 0)
        {
            list_event();
        }
#endif /* RT_USING_EVENT */
#ifdef RT_USING_MUTEX
        else if(strcmp(argv[1], "mutex") == 0)
        {
            list_mutex();
        }
#endif /* RT_USING_MUTEX */
#ifdef RT_USING_MAILBOX
        else if(strcmp(argv[1], "mailbox") == 0)
        {
            list_mailbox();
        }
#endif  /* RT_USING_MAILBOX */
#ifdef RT_USING_MESSAGEQUEUE
        else if(strcmp(argv[1], "msgqueue") == 0)
        {
            list_msgqueue();
        }
#endif /* RT_USING_MESSAGEQUEUE */
#ifdef RT_USING_MEMHEAP
        else if(strcmp(argv[1], "memheap") == 0)
        {
            list_memheap();
        }
#endif /* RT_USING_MEMHEAP */
#ifdef RT_USING_MEMPOOL
        else if(strcmp(argv[1], "mempool") == 0)
        {
            list_mempool();
        }
#endif /* RT_USING_MEMPOOL */
#ifdef RT_USING_DEVICE
        else if(strcmp(argv[1], "device") == 0)
        {
            list_device();
        }
#endif /* RT_USING_DEVICE */
#ifdef RT_USING_DFS
        else if(strcmp(argv[1], "fd") == 0)
        {
            extern int list_fd(void);
            list_fd();
        }
#endif /* RT_USING_DFS */
        else{
            goto _usage;
        }
        return 0;
    }
_usage:
    rt_kprintf("Usage: list [options]\n\r");
    rt_kprintf("[options]:\n\r");
    rt_kprintf("    %-12s - list threads\n\r", "thread");
    rt_kprintf("    %-12s - list timers\n\r", "timer");
#ifdef RT_USING_SEMAPHORE
    rt_kprintf("    %-12s - list semaphores\n\r", "sem");
#endif /* RT_USING_SEMAPHORE */
#ifdef RT_USING_MUTEX
    rt_kprintf("    %-12s - list mutexs\n\r", "mutex");
#endif /* RT_USING_MUTEX */
#ifdef RT_USING_EVENT
    rt_kprintf("    %-12s - list events\n\r", "event");
#endif /* RT_USING_EVENT */
#ifdef RT_USING_MAILBOX
    rt_kprintf("    %-12s - list mailboxs\n\r", "mailbox");
#endif /* RT_USING_MAILBOX */
#ifdef RT_USING_MESSAGEQUEUE
    rt_kprintf("    %-12s - list message queues\n\r", "msgqueue");
#endif /* RT_USING_MESSAGEQUEUE */
#ifdef RT_USING_MEMHEAP
    rt_kprintf("    %-12s - list memory heaps\n\r", "memheap");
#endif /* RT_USING_MEMHEAP */
#ifdef RT_USING_MEMPOOL
    rt_kprintf("    %-12s - list memory pools\n\r", "mempool");
#endif /* RT_USING_MEMPOOL */
#ifdef RT_USING_DEVICE
    rt_kprintf("    %-12s - list devices\n\r", "device");
#endif /* RT_USING_DEVICE */
#ifdef RT_USING_DFS
    rt_kprintf("    %-12s - list file descriptors\n\r", "fd");
#endif /* RT_USING_DFS */
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_list, list, list objects);

#endif /* RT_USING_FINSH */
