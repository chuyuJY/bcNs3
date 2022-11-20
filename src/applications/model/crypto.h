#ifndef CRYPTO_H
#define CRYPTO_H
#include <algorithm>
#include <fstream>
#include <bits/stdc++.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include "openssl/des.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"

#define KEY_LENGTH 1024 // 密钥长度
const std::string KeysPath = "keys";

namespace ns3 {

std::string sha256(const std::string &srcStr);

void getRSAKeyPair(int nodeId, RSA *pubKey, RSA *priKey);

void generateRSAKey(std::string publicFile, std::string privateFile); // 生成RSA到指定文件目录

void getPubKey(int nodeId, RSA *pubKey);

void getPriKey(int nodeId, RSA *priKey);

std::string sign(std::string &digest, RSA *priKey);

bool verifySign(std::string &digest, RSA *pubKey, std::string &signature);

} // namespace ns3

#endif // CRYPTO_H