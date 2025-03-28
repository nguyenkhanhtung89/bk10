#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ql_oe.h>
#include "aes.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/engine.h>
#include "log.h"
#include "encrypt.h"

#if (USE_ENCRYPT == 1)
void hex_log_debug(u8 *hex, u16 len){
	int i;
	for(i = 0; i < len; i++){
		printf("%02X ", hex[i]);
	}
	printf("\n\n\n\n");
}

void Encryption_DataStream(struct AES_ctx ctx,uint8_t *in,uint8_t *iv_t, uint8_t *out,int length){
	int i=0;
	uint8_t iv[16];
	memcpy(iv,iv_t,16);
	int j,red,block;
	block=length/16;
	red=length%16;
	for(i=0;i<block;i++){
		AES_encrypt(&ctx,iv);
		for(j=0;j<16;j++){
			out[i*16+j]=iv[j]^in[i*16+j];
		}
	}

	if(red!=0){
		AES_encrypt(&ctx,iv);
		for(j=0;j<red;j++){
			out[(i)*16+j]=iv[j]^in[((i)*16+j)];
		}
	}
}

int encrypt_file(char *filein, char *fileout, uint8_t key[], uint8_t iv_t[], RSA *rsa){
	u8  *file_stream_in;
	u8  *file_stream_out;
	FILE *FILE_IN, *FILE_OUT;
	u8 buff[256];
	u8 cipher[256];
	int cipher_len = 0;
	long filelen = 0;
	u8 key_iv[48];
	struct AES_ctx ctx;


	AES_init_ctx(&ctx, key);
	memcpy(key_iv, key, 32);
	memcpy(key_iv + 32, iv_t, 16);

	if(RSA_padding_add_PKCS1_OAEP(buff, 256, (const unsigned char *)key_iv, 48, NULL, 0) == 0){
		//printf("KHONG THUC HIEN PADDING NGAU NHIEN DUOC\n");
		return -1;
	}

	cipher_len = RSA_public_encrypt(256, buff, cipher, rsa, RSA_NO_PADDING);

	if(!(FILE_IN = fopen(filein, "rb"))){
		//printf("\n KHONG MO DUOC FILE: %s\n", filein);
		return -1;
	}else{
		fseek(FILE_IN, 0L, SEEK_END);
		filelen = ftell(FILE_IN);

		file_stream_in = malloc(filelen);
		file_stream_out = malloc(filelen);

		//printf("DO DAI FILE %ld \n", filelen);
		fseek(FILE_IN, 0L, SEEK_SET);
		fread(file_stream_in, 1, filelen, FILE_IN);
		Encryption_DataStream(ctx, file_stream_in, iv_t, file_stream_out, filelen);
		fclose(FILE_IN);
	}

	if(!(FILE_OUT = fopen(fileout, "wb"))){
		//printf("\n KHONG MO DUOC FILE: %s\n", fileout);
		return -1;
	}else{
		//my_print_t("CIPHER ->>>>>>>", cipher, 256);
		fseek(FILE_OUT, 0L, SEEK_SET);
		fwrite(cipher, 1, 256, FILE_OUT);
		fwrite(file_stream_out, 1, filelen, FILE_OUT);

		fclose(FILE_OUT);
	}

	return 0;
}

const char* public_exp = "010001";
unsigned char * priv_exp="00e5b0468a1ba11d41a62d6d9f5be515f72e909089479cbc233dfbd1d16b5fa32a74dbb8b0b944e4af072eb326a6a09c4688c0f94912eef2cd5f3f2b072e17a07a8d5ca9dc0c73ff0721de9f2e8922483c436462515ac1302b0c71e653c9f8d80e29dc1ccdc21e966b19597ae90187d9c61ea2f2cb708a307f422167a0dd17669e8a0980c9800497c49f607d9213d924d2b9739161fc99e391f2758b78b1034756361662e0b6bb4c179e15d8d6e1ddf60aac83786a59eb24705401bcc7d7497b90104a8b425635295fb6073e12cd2fe2c8608701e712b1a48dbae44716d041d1e9cc1418afeeae999c9a87976e462dafa00e00eb97fa7222ce4dd3f3c3892d8b19";
unsigned char *mod="00e864e896b2bb3a2df02bf6a8fc8412fcc8f5eb179c6da2146ae53e284102902b38ace01f8fe7f6675b8421a1745b9b943c42e11c19dd0193019f5ea61374b7fdd76a69f822e7f1e118e9e94d994b1aecb024f3b320408b0a18aa5de1206714db1609e00ecaf4742b792b3efbf66c59d15175894906b3938ea9447fa6397f68f63fd8906c8cb9a7dcde5b8673cd2a64c8ef8f775a35a3d8cca8d1be9e4cc6a166c465c0e082a8abc6a86fb99d8dffab0a5ea2429a4b265e3e8347cb8d0cc22dc69f3f78b0ec82955ff09dce0cb8503c0920ef4e2a63d5f83f1a206d4fba875087b1b2594834b3df41dcdd217fc27b83507e06de710dd8911aa888bae24648b469";

int encrypt_picture(char *InputName, char *OutPutName){
	uint8_t key_file[32];
	uint8_t iv_file[32];
	unsigned char buffkey[48];

	RAND_bytes(buffkey, 48);
	//=====================MA FILE ===============================================

	//my_print_t("GIA TRI KEY SINH NGAU NHIEN 48 bytes",buffkey,48);

	RSA *rsa = RSA_new();
	rsa->p = NULL;
	rsa->q = NULL;
	rsa->dmp1 = NULL;
	rsa->dmp1 = NULL;
	rsa->iqmp = NULL;

	BIGNUM *public_exp_bn = BN_new();
	BN_hex2bn(&public_exp_bn, public_exp);
	rsa->e = public_exp_bn;
	BIGNUM *priv_exp_bn = BN_new();
	BN_hex2bn(&priv_exp_bn, priv_exp);
	rsa->d = priv_exp_bn;
	BIGNUM *mod_bn = BN_new();
	BN_hex2bn(&mod_bn, mod);
	rsa->n = mod_bn;

	memcpy(key_file, buffkey, 32);
	memcpy(iv_file, buffkey+32, 16);

	encrypt_file(InputName, OutPutName, key_file, iv_file, rsa);

	RSA_free(rsa);
	//============================================================================
	return 0;
}

void encrypt_GPSdata(u8 *IMEI, u32 time, u8* IN, u8* OUT, int len){
	struct AES_ctx ctx;
	u8 pass[33];
	u8 salt[30];
	u8 result[64];

	u8 key[32];
	u8 iv[16];
	//u8 decrypt[128];
	//u8 encrypt[128];

	memcpy(salt,  IMEI, 15);
	memcpy(pass, IMEI, 15);
	pass[15] = (time >> 24) & 0xFF;
	pass[16] = (time >> 16) & 0xFF;
	pass[17] = (time >> 8) & 0xFF;
	pass[18] = time & 0xFF;

//	log_system("IMEI = %s, TIME = %u\n", IMEI, time);

	PKCS5_PBKDF2_HMAC(pass, 19, salt, 15, 1000, EVP_sha256(), 64, result);

//	log_system("KET QUA SAU KHI DAN XUAT\n");
//	hex_log_debug(result, 64);

	memcpy(key,result,32);

//	log_system("KEY SAU KHI DAN XUAT\n");
//	hex_log_debug(key, 32);

	memcpy(iv,result+32,16);

//	log_system("IV SAU KHI DAN XUAT\n");
//	hex_log_debug(iv,16);

	AES_init_ctx(&ctx, key);

//	log_system("TRUOC KHI MA HOA\n");
//	hex_log_debug(IN,len);

	Encryption_DataStream(ctx, IN, iv, OUT, len);

//	log_system("SAU KHI MA HOA\n");
//	hex_log_debug(OUT,len);

//	Encryption_DataStream(ctx, OUT, iv, decrypt, len);

//	log_system("SAU KHI GIAI MA\n");
//	hex_log_debug(decrypt,len);
}

#endif /* USE_ENCRYPT */

