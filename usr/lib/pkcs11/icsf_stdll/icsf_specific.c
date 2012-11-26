/*
 * Licensed materials, Property of IBM Corp.
 *
 * openCryptoki ICSF token
 *
 * (C) COPYRIGHT International Business Machines Corp. 2001, 2002, 2006, 2012
 *
 * Author: Marcelo Cerri (mhcerri@br.ibm.com)
 *
 * Based on CCC token.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "pkcs11types.h"
#include "defs.h"
#include "host_defs.h"
#include "h_extern.h"
#include "tok_specific.h"
#include "tok_struct.h"

/* Default token attributes */
CK_CHAR manuf[] = "IBM Corp.";
CK_CHAR model[] = "IBM ICSFTok ";
CK_CHAR descr[] = "IBM PKCS#11 ICSF token";
CK_CHAR label[] = "IBM OS PKCS#11   ";

/* mechanisms provided by this token */
MECH_LIST_ELEMENT mech_list[] = {
  { CKM_RSA_PKCS_KEY_PAIR_GEN, {512,	4096,	CKF_HW | CKF_GENERATE_KEY_PAIR} },
  { CKM_DES_KEY_GEN, 	{8,	8,	CKF_HW | CKF_GENERATE} },
  { CKM_DES2_KEY_GEN, 	{24,	24,	CKF_HW | CKF_GENERATE} },
  { CKM_DES3_KEY_GEN, 	{24,	24,	CKF_HW | CKF_GENERATE} },
  { CKM_RSA_PKCS,	{512,	4096,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT |
			 		CKF_WRAP | CKF_UNWRAP |
			 		CKF_SIGN | CKF_VERIFY |
				 	CKF_SIGN_RECOVER | CKF_VERIFY_RECOVER} },
  { CKM_RSA_X_509,	{512,	4096,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT |
				 	CKF_SIGN | CKF_VERIFY |
				 	CKF_SIGN_RECOVER | CKF_VERIFY_RECOVER} },
  { CKM_MD2_RSA_PKCS,	{512,	4096,	CKF_SIGN | CKF_VERIFY} },
  { CKM_MD5_RSA_PKCS,	{512,	4096,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_SHA1_RSA_PKCS,	{512,	4096,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_SHA256_RSA_PKCS,	{512,	4096,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_SHA384_RSA_PKCS,	{512,	4096,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_SHA512_RSA_PKCS,	{512,	4096,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_DES_ECB,	{0,	0,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT} },
  { CKM_DES_CBC,	{0,	0,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT} },
  { CKM_DES_CBC_PAD,	{0,	0,	CKF_HW | CKF_ENCRYPT	| CKF_DECRYPT |
				 	CKF_WRAP | CKF_UNWRAP} },
  { CKM_DES3_ECB,	{0,	0,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT} }, 
  { CKM_DES3_CBC,	{0,	0,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT} },
  { CKM_DES3_CBC_PAD,	{0,	0,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT |
				 	CKF_WRAP | CKF_UNWRAP} },
  { CKM_SHA_1,		{0,	0,	CKF_HW | CKF_DIGEST } },
  { CKM_SHA256,		{0,	0,	CKF_HW | CKF_DIGEST } },
  { CKM_SHA384,		{0,	0,	CKF_HW | CKF_DIGEST } },
  { CKM_SHA512,		{0,	0,	CKF_HW | CKF_DIGEST } },
  { CKM_RIPEMD160,	{0,	0,	CKF_DIGEST } },
  { CKM_MD2,		{0,	0,	CKF_DIGEST } },
  { CKM_MD5,		{0,	0,	CKF_DIGEST } },
  { CKM_AES_KEY_GEN, 	{16,	32,	CKF_HW | CKF_GENERATE} },
  { CKM_AES_ECB,	{16,	32,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT} },
  { CKM_AES_CBC,	{16,	32,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT} },
  { CKM_AES_CBC_PAD,	{16,	32,	CKF_HW | CKF_ENCRYPT | CKF_DECRYPT |
				 	CKF_WRAP | CKF_UNWRAP} },
  { CKM_DSA_KEY_PAIR_GEN, 	{512,	2048,	CKF_HW | CKF_GENERATE_KEY_PAIR} },
  { CKM_DH_PKCS_KEY_PAIR_GEN, 	{512,	2048,	CKF_GENERATE_KEY_PAIR} },
  { CKM_EC_KEY_PAIR_GEN,	{160, 	521,	CKF_HW | CKF_GENERATE_KEY_PAIR |
					CKF_EC_F_P | CKF_EC_NAMEDCURVE |
					CKF_EC_UNCOMPRESS} },
  { CKM_SSL3_PRE_MASTER_KEY_GEN,	{48,	48,	CKF_HW | CKF_GENERATE} },
  { CKM_DSA_SHA1,	{512,	4096,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_DSA,		{512,	2048,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_ECDSA_SHA1,	{512,	4096,	CKF_HW | CKF_SIGN | CKF_VERIFY |
					CKF_EC_F_P | CKF_EC_NAMEDCURVE |
					CKF_EC_UNCOMPRESS} },
  { CKM_ECDSA,		{160,	521,	CKF_HW | CKF_SIGN | CKF_VERIFY |
					CKF_EC_F_P | CKF_EC_NAMEDCURVE |
					CKF_EC_UNCOMPRESS} },
  { CKM_MD5_HMAC,	{0,	0,	CKF_SIGN | CKF_VERIFY} },
  { CKM_SHA_1_HMAC,	{0,	0,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_SHA256_HMAC,	{0,	0,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_SHA384_HMAC,	{0,	0,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_SHA512_HMAC,	{0,	0,	CKF_HW | CKF_SIGN | CKF_VERIFY} },
  { CKM_SSL3_MD5_MAC,	{384,	384,	CKF_SIGN | CKF_VERIFY} },
  { CKM_SSL3_SHA1_MAC,	{384,	384,	CKF_SIGN | CKF_VERIFY} },
  { CKM_DH_PKCS_DERIVE,	{512,	2048,	CKF_DERIVE} },
  { CKM_SSL3_MASTER_KEY_DERIVE, {48,	48,	CKF_DERIVE} },
};

CK_ULONG mech_list_len = (sizeof(mech_list) / sizeof(MECH_LIST_ELEMENT));

CK_RV
token_specific_get_mechanism_list(CK_MECHANISM_TYPE_PTR pMechanismList,
					CK_ULONG_PTR pulCount)
{
	int rc;
	rc = ock_generate_get_mechanism_list(pMechanismList, pulCount);
	return rc;
}

CK_RV
token_specific_get_mechanism_info(CK_MECHANISM_TYPE type,
				  CK_MECHANISM_INFO_PTR pInfo)
{
	int rc;
	/* common/mech_list.c */
	rc = ock_generate_get_mechanism_info(type, pInfo);
	return rc;
}

/*
 * Convert pkcs slot number to local representation
 */
int
tok_slot2local(CK_SLOT_ID snum)
{
	return 1;
}

/*
 * Called during C_Initialize.
 */
CK_RV
token_specific_init(char *correlator, CK_SLOT_ID slot_id, char *conf_name)
{
	return CKR_OK;
}

/*
 * Called during C_Finalize.
 */
CK_RV
token_specific_final()
{
	return CKR_OK;
}

/*
 * Initialize the shared memory region. ICSF has to use a custom method for
 * this because it uses additional data in the shared memory and in the future
 * multiple slots should be supported for ICSF.
 */
CK_RV
token_specific_attach_shm(CK_SLOT_ID slot_id, LW_SHM_TYPE **shm)
{
	CK_RV rc = CKR_OK;
	int ret;

	XProcLock();

	/*
	 * Attach to an existing shared memory region or create it if it doesn't
	 * exists. When the it's created (ret=0) the region is initialized with
	 * zeroes.
	 */
	ret = sm_open(pk_dir, 0666, (void**) shm, sizeof(**shm), 0);
	if (ret < 0) {
		OCK_LOG_ERR((rc = CKR_FUNCTION_FAILED));
		goto done;
	}

done:
	XProcUnLock();
	return rc;
}

/*
 * Initialize token.
 */
CK_RV
token_specific_init_token(CK_SLOT_ID sid, CK_CHAR_PTR pPin, CK_ULONG ulPinLen,
			  CK_CHAR_PTR pLabel)
{
	CK_RV rc = CKR_OK;

	OCK_LOG_DEBUG("dir %s\n", pk_dir);

	return CKR_OK;
}
