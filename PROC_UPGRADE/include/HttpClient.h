/*
 *   File name: HttpClient.h
 *
 *  Created on: 2018年6月15日
 *      Author: Cary
 *     Version: 1.0
 */
#ifndef SRC_HTTPCLIENT_H_
#define SRC_HTTPCLIENT_H_

#include <string>


/**
	 * @brief HTTP POST请求
	 * @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
	 * @param strPost 输入参数,使用如下格式para1=val1¶2=val2&…
	 * @param strResponse 输出参数,返回的内容
	 * @return 返回是否Post成功
	 */
int Post(const std::string &strUrl, const std::string &strPost,
         std::string &strResponse);

/**
	 * @brief HTTP GET请求
	 * @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
	 * @param strResponse 输出参数,返回的内容
	 * @return 返回是否Post成功
	 */
int Get(const std::string &strUrl, std::string &strResponse);

/**
	 * @brief HTTPS POST请求,无证书版本
	 * @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
	 * @param strPost 输入参数,使用如下格式para1=val1¶2=val2&…
	 * @param strResponse 输出参数,返回的内容
	 * @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
	 * @return 返回是否Post成功
	 */
int Posts(const std::string &strUrl, const std::string &strPost,
          std::string &strResponse, const char *pCaPath = NULL);

/**
	 * @brief HTTPS GET请求,无证书版本
	 * @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
	 * @param strResponse 输出参数,返回的内容
	 * @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
	 * @return 返回是否Post成功
	 */
int Gets(const std::string &strUrl, std::string &strResponse,
         const char *pCaPath = NULL);

#endif /* SRC_HTTPCLIENT_H_ */
