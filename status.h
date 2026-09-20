#ifndef _STATUS_H_
#define _STATUS_H_

typedef enum
{
    STATUS_SUCCESS = 0,
    STATUS_INPROGRESS,
    STATUS_EFAILURE,
    STATUS_EINVALIDPARAMS,
    STATUS_EFILEERROR,
    NUM_STATUS,
} status_t;

#endif