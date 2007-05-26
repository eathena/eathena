// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _NULLPO_H_
#define _NULLPO_H_

#include "basetypes.h"

// 全体のスイッチを宣言しているヘッダがあれば
// そこに移動していただけると
#define NULLPO_CHECK 1


#define NLP_MARK __FILE__, __LINE__, __func__

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
 * nullpo_break(t)
 *   戻り値 なし
 * [引数]
  *  t       チェック対象
 *--------------------------------------
 */

#if NULLPO_CHECK

#define nullpo_retv(t)		if (nullpo_chk(NLP_MARK, (void *)(t))) {return;}

#define nullpo_retr(ret, t)	if (nullpo_chk(NLP_MARK, (void *)(t))) {return(ret);}

#define nullpo_break(t)		if (nullpo_chk(NLP_MARK, (void *)(t))) {break;}


#else
// No Nullpo check 

// if((t)){;}
// 良い方法が思いつかなかったので・・・苦肉の策です。
// 一応ワーニングは出ないはず


#define nullpo_retv(t)		if((t)){;}
#define nullpo_retr(ret, t)	if((t)){;}
#define nullpo_break(t)		if((t)){;}



#endif // NULLPO_CHECK

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



/*======================================
 * nullpo_info
 *   nullpo情報出力
 * [引数]
 *  file    __FILE__
 *  line    __LINE__
 *  func    __func__ (関数名)
 *    これらには NLP_MARK を使うとよい
 *--------------------------------------
 */
void nullpo_info(const char *file, int line, const char *func);





#endif
