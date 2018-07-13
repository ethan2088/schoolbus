
/**
 * INI�����ļ���������
 * Ini file parse functions.
 * By Hoverlees http://www.hoverlees.com me[at]hoverlees.com
 */
#ifndef _HOVERLEES_INI_CONFIG_H
#define _HOVERLEES_INI_CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
typedef struct _CONFIG_BTREE_NODE
{
	char* key;
	void* data;
	struct _CONFIG_BTREE_NODE* left;
	struct _CONFIG_BTREE_NODE* right;
	char mem[2];
} CONFIG_BTREE_NODE;
typedef struct _CONFIG_BTREE
{
	int numNodes;
	CONFIG_BTREE_NODE* root;
} CONFIG_BTREE;
typedef CONFIG_BTREE INI_CONFIG;
typedef int (*CONFIG_BTREE_TRAVERSE_CB)(CONFIG_BTREE_NODE* node);
typedef int (*CONFIG_BTREE_SAVE_TRAVERSE_CB)(FILE* fp,CONFIG_BTREE_NODE* node);
/**
 * ini���ݽ�������,���ַ�����������
 * @param str �ַ���
 * @param slen �ַ�������,����Ϊ0,���Ϊ��,�����Զ������ַ�������
 * @param isGBK ���ini�ļ�ʹ��GBK�ַ���,���ó�1,�������ó�0
 * @return �ɹ�����INI_CONFIGָ��,ʧ�ܷ���null
 */
INI_CONFIG* ini_config_create_from_string(char* str,int slen,int isGBK);
/**
 * ini���ݽ�������,���ļ���������
 * @param filename �����ļ���
 * @param isGBK ���ini�ļ�ʹ��GBK�ַ���,���ó�1,�������ó�0
 * @return �ɹ�����INI_CONFIGָ��,ʧ�ܷ���null
 */
INI_CONFIG* ini_config_create_from_file(const char* filename,int isGBK);
/**
 * �����ͷź���,�ͷ���ռ�õ��ڴ漰���ݽṹ
 * @param config ���ö���ָ��
 * @return �ɹ����أ�,ʧ�ܷ��أ�
 */
void ini_config_destroy(INI_CONFIG* config);
/**
 * ��ȡ��������ֵ
 * @param config ���ö���ָ��
 * @param section ����,û�ж���ʱ����ΪNULL
 * @param key ����
 * @param default_int��Ĭ��ֵ
 * @return ����������д˼���Ӧ��ֵ,���ظ�ֵ,���򷵻ز���ָ����Ĭ��ֵ��
 */
int ini_config_get_int(INI_CONFIG* config,const char* section,const char* key,int default_int);
/**
 * ��ȡ�����ַ���ֵ
 * @param config ���ö���ָ��
 * @param section ����,û�ж���ʱ����ΪNULL
 * @param key ����
 * @param default_string��Ĭ��ֵ
 * @return ����������д˼���Ӧ��ֵ,���ظ�ֵ,���򷵻ز���ָ����Ĭ��ֵ��
 */
char* ini_config_get_string(INI_CONFIG* config,const char* section,const char* key,char* default_string);
/**
 * ���ñ���
 * @param config ���ö���ָ��
 * @param section������,û�ж���ʱ����ΪNULL
 * @param key������
 * @param key_len������
 * @param value��ֵ
 * @param value_len��ֵ����
 * @return �ɹ�Ϊ��,ʧ��Ϊ��
 */
int ini_config_set_string(INI_CONFIG* config,const char* section,const char* key,int key_len,const char* value,int value_len);
/**
 * ���ñ���
 * @param config ���ö���ָ��
 * @param section������,û�ж���ʱ����ΪNULL
 * @param key������
 * @param key_len������
 * @param value������ֵ
 * @return �ɹ�Ϊ��,ʧ��Ϊ��
 */
int ini_config_set_int(INI_CONFIG* config,const char* section,const char* key,int key_len,int value);
/**
 * �������õ��ļ��С�*��ʾ,ԭ�������ļ��е�ע����Ϣ�����ᱣ��.
 * @param config ���ö���ָ��
 * @param filename ���浽���ļ�
 * @return �ɹ�Ϊ��,ʧ��Ϊ0
 */
int ini_config_save(INI_CONFIG* config,const char* filename);
/**
 * ������ini_config_save,ֻ�ǲ������ļ�ָ��,�˺�������ֱ��ʹ��stdin,stdout,stderr.��*��ʾ:������������ر�fp.
 * @param config ���ö���ָ��
 * @param fp �ļ�ָ��
 * @return �ɹ�Ϊ��,ʧ��Ϊ0
 */
int ini_config_print(INI_CONFIG* config,FILE* fp);
#endif

