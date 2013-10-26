// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _NULLPO_H_
#define _NULLPO_H_

#include "cbasetypes.h"

#define NLP_MARK __FILE__, __LINE__, __func__

// enabled by default on debug builds
#if defined(DEBUG) && !defined(NULLPO_CHECK)
	#define NULLPO_CHECK
#endif

/*----------------------------------------------------------------------------
 * Macros
 *----------------------------------------------------------------------------
 */
/*======================================
 * Nullチェック 及び 情報出力後 return
 *・展開するとifとかreturn等が出るので
 *  一行単体で使ってください。
 *・nullpo_ret(x = func());
 *  のような使用法も想定しています。
 *--------------------------------------
 * nullpo_ret(t)
 *   戻り値 0固定
 * [引数]
 *  t       チェック対象
 *--------------------------------------
 * nullpo_retv(t)
 *   戻り値 なし
 * [引数]
 *  t       チェック対象
 *--------------------------------------
 * nullpo_retr(ret, t)
 *   戻り値 指定
 * [引数]
 *  ret     return(ret);
 *  t       チェック対象
 *--------------------------------------
 */

#if defined(NULLPO_CHECK)

#define nullpo_ret(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return(0);}

#define nullpo_retv(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return;}

#define nullpo_retr(ret, t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {return(ret);}

#define nullpo_retb(t) \
	if (nullpo_chk(NLP_MARK, (void *)(t))) {break;}

#else /* NULLPO_CHECK */
/* No Nullpo check */

// if((t)){;}
// 良い方法が思いつかなかったので・・・苦肉の策です。
// 一応ワーニングは出ないはず

#define nullpo_ret(t) (void)(t)
#define nullpo_retv(t) (void)(t)
#define nullpo_retr(ret, t) (void)(t)
#define nullpo_retb(t) (void)(t)

#endif /* NULLPO_CHECK */

/*----------------------------------------------------------------------------
 * Functions
 *----------------------------------------------------------------------------
 */
/*======================================
 * nullpo_chk
 *   Nullチェック 及び 情報出力
 * [引数]
 *  file    __FILE__
 *  line    __LINE__
 *  func    __func__ (関数名)
 *    これらには NLP_MARK を使うとよい
 *  target  チェック対象
 * [返り値]
 *  0 OK
 *  1 NULL
 *--------------------------------------
 */
int nullpo_chk(const char *file, int line, const char *func, const void *target);

#endif /* _NULLPO_H_ */
