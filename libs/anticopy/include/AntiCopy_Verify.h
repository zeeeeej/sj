#ifndef __ANTICOPY_VERIFY_H__
#define __ANTICOPY_VERIFY_H__

/*
 *#define ANTICOPY_PREV_STRING "xxxxxxxxxxxxxxxxxxxx"
 *#define PREV_LEN              32
 */

#ifndef ANTICOPY_PREV_STRING
int AntiCopy_Verify(void);
#else
int AntiCopy_Verify_PrevStr(char *prev);
#endif

#endif /* __ANTICOPY_VERIFY_H__ */
