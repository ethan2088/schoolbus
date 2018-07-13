
#ifndef _RECORD_H
#define _RECORD_H

int RecordInit(void);
int RecordPush(UPLOAD_DATA *prec);
int RecordPop(UPLOAD_DATA *prec);
int RecordDel(uint32_t tagid);
int RecordGet(void);

#endif //_RECORD_H
