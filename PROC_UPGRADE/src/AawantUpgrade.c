/********************************************************
* FILE     : AawantUpgrade.c
* CONTENT  : 升级服务主程序
*********************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "AI_PKTHEAD.h"
#include "AawantData.h"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AIprofile.h"
#include "AIUComm.h"
#include "cJSON.h"
#include <upg_control.h>
#include "upg_download.h"
#include "HttpClient.h"

int server_sock;        // 服务器SOCKET
DOWNLOAD_PARAM dl_param;
int status;
/*
//空闲
#define AAWANT_SYSTEM_IDLE_TASK 601

//播放音乐
#define AAWANT_SYSTEM_AUDIO_TASK 602

//播放TTS
#define AAWANT_SYSTEM_TTS_TASK 603

//播放闹铃
#define AAWANT_SYSTEM_ALARM_TASK 604

//系统正在配置网络
#define AAWANT_SYSTEM_NETCONFIG_TASK 605

//系统正在拾音
#define AAWANT_SYSTEM_MSC_RECOGNIZE 606

//系统正在请求服务器
#define AAWANT_SYSTEM_REQUEST_SERVLET 607

//系统正在执行指令动作
#define AAWANT_SYSTEM_COMMAND_CONTROL 608

*/

/*
 * {
        "nowVersion":1，
        "toVersion":10,
                "url":"http://www.aawant.com/ZFERq9ymV7SnMAUP/group1/M00/12/0D/
        CgoKEVtYXVmAPAmmFetTC6kgkm0083.zip ",
                                           "model":"mtk6735_release_L"
    }
 */


#if 0
void *Do_Download(void *dl) {
    // dl_param.dl_sock=server_sock;
    Aawant_Set_Upgrade_Status(&dl_param, AAW_CTL_DOWNLOAD_DOING);
    //  int ret=Aawant_StartDownLoad(a_dl_param,"http://192.168.1.118/","/home/sine/download",True);
    int ret = Aawant_StartDownLoad(dl_param, "http://192.168.1.118/", "/home/sine/download", True);

    if (ret == -1) {
        printf("[%s]==>failed\n", __FUNCTION__);
        Aawant_Set_Upgrade_Status(&dl_param, AAW_CTL_DOWNLOAD_FAIL);
        FROM_UPGRADE_DATA upgradeData;
        upgradeData.status = DOWNLOAD_FAIL;

        //  printf("[%s]==>sock=%d,errcode=%d\n",__FUNCTION__,a_dl_param.dl_sock,a_dl_param.errcode);
        printf("[%s]==>sock=%d,errcode=%d\n", __FUNCTION__, dl_param.dl_sock, dl_param.errcode);

        AAWANTSendPacket(server_sock, PKT_UPGRADE_FEEDBACK, (char *) &upgradeData, sizeof(upgradeData));
    } else {
        printf("[%s]==>sucess\n", __FUNCTION__);
        Aawant_Set_Upgrade_Status(&dl_param, AAW_CTL_DOWNLOAD_SUCESS);
        FROM_UPGRADE_DATA upgradeData;
        upgradeData.status = DOWNLOAD_SUCESS;
        upgradeData.code = 0;

        printf("[%s]==>sock=%d,errcode=%d\n", __FUNCTION__, dl_param.dl_sock, dl_param.errcode);
        AAWANTSendPacket(server_sock, PKT_UPGRADE_FEEDBACK, (char *) &upgradeData, sizeof(upgradeData));
    }

    printf("-----------------[%s][End]---------------\n", __FUNCTION__);

}
#endif

void *Do_Download2(void *arg) {
    Aawant_Set_Upgrade_Status(&dl_param, AAW_CTL_DOWNLOAD_DOING);
    //  int ret=Aawant_StartDownLoad(a_dl_param,"http://192.168.1.118/","/home/sine/download",True);
    int ret = Aawant_StartDownLoad2(dl_param, dl_param.url, dl_param.save_path);

    if (ret == -1) {
        printf("[%s]==>failed\n", __FUNCTION__);
        Aawant_Set_Upgrade_Status(&dl_param, AAW_CTL_DOWNLOAD_FAIL);

    } else {
        printf("[%s]==>sucess\n", __FUNCTION__);
        Aawant_Set_Upgrade_Status(&dl_param, AAW_CTL_DOWNLOAD_SUCESS);
    }

    printf("-----------------[%s][End]---------------\n", __FUNCTION__);
}

void parseJson(){

}

int check_CurrentVersion(){
    return 0;
}

/**
 * 检查升级结果
 * @return
 */
int check_Upgrade_Result() {
}


/**
 * 上报升级结果
 */
void report_UpgradeResult(){

}


/**
 *
 * @return
 */
int32 createDownloadPthread() {
    pthread_t dl_ptd;
    // DOWNLOAD_PARAM dl=arg;
    // dl.dl_sock=arg.dl_sock;

   // mprintf("[%s]==>%d\n", __FUNCTION__, arg.dl_sock);
    // int32 ret=pthread_create(&dl_ptd,NULL,Do_Download,&dl);
    int32 ret = pthread_create(&dl_ptd, NULL, Do_Download2, NULL);

    if (ret != 0) {
        printf("[%s]==>create pthread fail\n", __FUNCTION__);
    }


    return ret;
}


#ifdef jiexi

typedef enum {
CURLE_OK = 0,
CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
CURLE_FAILED_INIT,             /* 2 */
CURLE_URL_MALFORMAT,           /* 3 */
CURLE_NOT_BUILT_IN,            /* 4 - [was obsoleted in August 2007 for
                                    7.17.0, reused in April 2011 for 7.21.5] */
CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
CURLE_COULDNT_CONNECT,         /* 7 */
CURLE_FTP_WEIRD_SERVER_REPLY,  /* 8 */
CURLE_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
                                    due to lack of access - when login fails
                                    this is not returned. */
CURLE_FTP_ACCEPT_FAILED,       /* 10 - [was obsoleted in April 2006 for
                                    7.15.4, reused in Dec 2011 for 7.24.0]*/
CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
CURLE_FTP_ACCEPT_TIMEOUT,      /* 12 - timeout occurred accepting server
                                    [was obsoleted in August 2007 for 7.17.0,
                                    reused in Dec 2011 for 7.24.0]*/
CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
CURLE_FTP_CANT_GET_HOST,       /* 15 */
CURLE_HTTP2,                   /* 16 - A problem in the http2 framing layer.
                                    [was obsoleted in August 2007 for 7.17.0,
                                    reused in July 2014 for 7.38.0] */
CURLE_FTP_COULDNT_SET_TYPE,    /* 17 */
CURLE_PARTIAL_FILE,            /* 18 */
CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
CURLE_OBSOLETE20,              /* 20 - NOT USED */
CURLE_QUOTE_ERROR,             /* 21 - quote command failure */
CURLE_HTTP_RETURNED_ERROR,     /* 22 */
CURLE_WRITE_ERROR,             /* 23 */
CURLE_OBSOLETE24,              /* 24 - NOT USED */
CURLE_UPLOAD_FAILED,           /* 25 - failed upload "command" */
CURLE_READ_ERROR,              /* 26 - couldn't open/read from file */
CURLE_OUT_OF_MEMORY,           /* 27 */
/* Note: CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
         instead of a memory allocation error if CURL_DOES_CONVERSIONS
         is defined
*/
CURLE_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
CURLE_OBSOLETE29,              /* 29 - NOT USED */
CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
CURLE_OBSOLETE32,              /* 32 - NOT USED */
CURLE_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
CURLE_HTTP_POST_ERROR,         /* 34 */
CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
CURLE_LDAP_CANNOT_BIND,        /* 38 */
CURLE_LDAP_SEARCH_FAILED,      /* 39 */
CURLE_OBSOLETE40,              /* 40 - NOT USED */
CURLE_FUNCTION_NOT_FOUND,      /* 41 */
CURLE_ABORTED_BY_CALLBACK,     /* 42 */
CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
CURLE_OBSOLETE44,              /* 44 - NOT USED */
CURLE_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
CURLE_OBSOLETE46,              /* 46 - NOT USED */
CURLE_TOO_MANY_REDIRECTS ,     /* 47 - catch endless re-direct loops */
CURLE_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
CURLE_TELNET_OPTION_SYNTAX ,   /* 49 - Malformed telnet option */
CURLE_OBSOLETE50,              /* 50 - NOT USED */
CURLE_PEER_FAILED_VERIFICATION, /* 51 - peer's certificate or fingerprint
                                     wasn't verified fine */
CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                    default */
CURLE_SEND_ERROR,              /* 55 - failed sending network data */
CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
CURLE_OBSOLETE57,              /* 57 - NOT IN USE */
CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
CURLE_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
CURLE_LDAP_INVALID_URL,        /* 62 - Invalid LDAP URL */
CURLE_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
CURLE_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
CURLE_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
                                    that failed */
CURLE_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
CURLE_LOGIN_DENIED,            /* 67 - user, password or similar was not
                                    accepted and we failed to login */
CURLE_TFTP_NOTFOUND,           /* 68 - file not found on server */
CURLE_TFTP_PERM,               /* 69 - permission problem on server */
CURLE_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
CURLE_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
CURLE_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
CURLE_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
CURLE_TFTP_NOSUCHUSER,         /* 74 - No such user */
CURLE_CONV_FAILED,             /* 75 - conversion failed */
CURLE_CONV_REQD,               /* 76 - caller must register conversion
                                    callbacks using curl_easy_setopt options
                                    CURLOPT_CONV_FROM_NETWORK_FUNCTION,
                                    CURLOPT_CONV_TO_NETWORK_FUNCTION, and
                                    CURLOPT_CONV_FROM_UTF8_FUNCTION */
CURLE_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
                                    or wrong format */
CURLE_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
CURLE_SSH,                     /* 79 - error from the SSH layer, somewhat
                                    generic so the error message will be of
                                    interest when this has happened */

CURLE_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
                                    connection */
CURLE_AGAIN,                   /* 81 - socket is not ready for send/recv,
                                    wait till it's ready and try again (Added
                                    in 7.18.2) */
CURLE_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
                                    wrong format (Added in 7.19.0) */
CURLE_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
                                    7.19.0) */
CURLE_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
CURLE_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
CURLE_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
CURLE_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
CURLE_CHUNK_FAILED,            /* 88 - chunk callback reported error */
CURLE_NO_CONNECTION_AVAILABLE, /* 89 - No connection available, the
                                    session will be queued */
CURLE_SSL_PINNEDPUBKEYNOTMATCH, /* 90 - specified pinned public key did not
                                     match */
CURLE_SSL_INVALIDCERTSTATUS,   /* 91 - invalid certificate status */
CURL_LAST /* never use! */
} CURLcode;

const char *curl_easy_strerror_cn(CURLcode code) {
    switch (code) {
        case CURLE_UNSUPPORTED_PROTOCOL:
            return "您传送给 libcurl 的网址使用了此 libcurl 不支持的协议。 可能是您没有使用的编译时选项造成了这种情况（可能是协议字符串拼写有误，或没有指定协议 libcurl 代码）。";

        case CURLE_FAILED_INIT:
            return "非常早期的初始化代码失败。 可能是内部错误或问题。";

        case CURLE_URL_MALFORMAT:
            return "网址格式不正确。";

        case CURLE_COULDNT_RESOLVE_PROXY:
            return "无法解析代理服务器。 指定的代理服务器主机无法解析。";

        case CURLE_COULDNT_RESOLVE_HOST:
            return "无法解析主机。 指定的远程主机无法解析。";

        case CURLE_COULDNT_CONNECT:
            return "无法通过 connect() 连接至主机或代理服务器。";

        case CURLE_FTP_WEIRD_SERVER_REPLY:
            return "在连接到 FTP 服务器后，libcurl 需要收到特定的回复。 此错误代码表示收到了不正常或不正确的回复。 指定的远程服务器可能不是正确的 FTP 服务器。";

        case CURLE_REMOTE_ACCESS_DENIED:
            return "我们无法访问网址中指定的资源。 对于 FTP，如果尝试更改为远程目录，就会发生这种情况。";

        case CURLE_FTP_WEIRD_PASS_REPLY:
            return "在将 FTP 密码发送到服务器后，libcurl 需要收到正确的回复。 此错误代码表示返回的是意外的代码。";

        case CURLE_FTP_WEIRD_PASV_REPLY:
            return "libcurl 无法从服务器端收到有用的结果，作为对 PASV 或 EPSV 命令的响应。 服务器有问题。";

        case CURLE_FTP_WEIRD_227_FORMAT:
            return "FTP 服务器返回 227 行作为对 PASV 命令的响应。 如果 libcurl 无法解析此行，就会返回此代码。";

        case CURLE_FTP_CANT_GET_HOST:
            return "在查找用于新连接的主机时出现内部错误。";

        case CURLE_FTP_COULDNT_SET_TYPE:
            return "在尝试将传输模式设置为二进制或 ascii 时发生错误。";

        case CURLE_PARTIAL_FILE:
            return "文件传输尺寸小于或大于预期。 当服务器先报告了一个预期的传输尺寸，然后所传送的数据与先前指定尺寸不相符时，就会发生此错误。";

        case CURLE_FTP_COULDNT_RETR_FILE:
            return "‘RETR’ 命令收到了不正常的回复，或完成的传输尺寸为零字节。";

        case CURLE_QUOTE_ERROR:
            return "在向远程服务器发送自定义 “QUOTE” 命令时，其中一个命令返回的错误代码为 400 或更大的数字（对于 FTP），或以其他方式表明命令无法成功完成。";

        case CURLE_HTTP_RETURNED_ERROR:
            return "如果 CURLOPT_FAILONERROR 设置为 TRUE，且 HTTP 服务器返回 >= 400 的错误代码，就会返回此代码。 （此错误代码以前又称为 CURLE_HTTP_NOT_FOUND。）";

        case CURLE_WRITE_ERROR:
            return "在向本地文件写入所收到的数据时发生错误，或由写入回调 (write callback) 向 libcurl 返回了一个错误。";

        case CURLE_UPLOAD_FAILED:
            return "无法开始上传。 对于 FTP，服务器通常会拒绝执行 STOR 命令。 错误缓冲区通常会提供服务器对此问题的说明。 （此错误代码以前又称为 CURLE_FTP_COULDNT_STOR_FILE。）";

        case CURLE_READ_ERROR:
            return "读取本地文件时遇到问题，或由读取回调 (read callback) 返回了一个错误。";

        case CURLE_OUT_OF_MEMORY:
            return "内存分配请求失败。 此错误比较严重，若发生此错误，则表明出现了非常严重的问题。";

        case CURLE_OPERATION_TIMEDOUT:
            return "操作超时。 已达到根据相应情况指定的超时时间。 请注意： 自 Urchin 6.6.0.2 开始，超时时间可以自行更改。 要指定远程日志下载超时，请打开 urchin.conf 文件，取消以下行的注释标记：#DownloadTimeout: 30";

        case CURLE_FTP_PORT_FAILED:
            return "FTP PORT 命令返回错误。 在没有为 libcurl 指定适当的地址使用时，最有可能发生此问题。 请参阅 CURLOPT_FTPPORT。";

        case CURLE_FTP_COULDNT_USE_REST:
            return "FTP REST 命令返回错误。 如果服务器正常，则应当不会发生这种情况。";

        case CURLE_RANGE_ERROR:
            return "服务器不支持或不接受范围请求。";

        case CURLE_HTTP_POST_ERROR:
            return "此问题比较少见，主要由内部混乱引发。";

        case CURLE_SSL_CONNECT_ERROR:
            return "同时使用 SSL/TLS 时可能会发生此错误。 您可以访问错误缓冲区查看相应信息，其中会对此问题进行更详细的介绍。 可能是证书（文件格式、路径、许可）、密码及其他因素导致了此问题。";

        case CURLE_FTP_BAD_DOWNLOAD_RESUME:
            return "尝试恢复超过文件大小限制的 FTP 连接。";

        case CURLE_FILE_COULDNT_READ_FILE:
            return "无法打开 FILE:// 路径下的文件。 原因很可能是文件路径无法识别现有文件。 建议您检查文件的访问权限。";

        case CURLE_LDAP_CANNOT_BIND:
            return "LDAP 无法绑定。LDAP 绑定操作失败。";

        case CURLE_LDAP_SEARCH_FAILED:
            return "LDAP 搜索无法进行。";

        case CURLE_FUNCTION_NOT_FOUND:
            return "找不到函数。 找不到必要的 zlib 函数。";

        case CURLE_ABORTED_BY_CALLBACK:
            return "由回调中止。 回调向 libcurl 返回了 “abort”。";

        case CURLE_BAD_FUNCTION_ARGUMENT:
            return "内部错误。 使用了不正确的参数调用函数。";

        case CURLE_INTERFACE_FAILED:
            return "界面错误。 指定的外部界面无法使用。 请通过 CURLOPT_INTERFACE 设置要使用哪个界面来处理外部连接的来源 IP 地址。 （此错误代码以前又称为 CURLE_HTTP_PORT_FAILED。）";

        case CURLE_TOO_MANY_REDIRECTS:
            return "重定向过多。 进行重定向时，libcurl 达到了网页点击上限。 请使用 CURLOPT_MAXREDIRS 设置上限。";

        case CURLE_UNKNOWN_TELNET_OPTION:
            return "无法识别以 CURLOPT_TELNETOPTIONS 设置的选项。 请参阅相关文档。";

        case CURLE_TELNET_OPTION_SYNTAX:
            return "telnet 选项字符串的格式不正确。";

        case CURLE_PEER_FAILED_VERIFICATION:
            return "远程服务器的 SSL 证书或 SSH md5 指纹不正确。";

        case CURLE_GOT_NOTHING:
            return "服务器未返回任何数据，在相应情况下，未返回任何数据就属于出现错误。";

        case CURLE_SSL_ENGINE_NOTFOUND:
            return "找不到指定的加密引擎。";

        case CURLE_SSL_ENGINE_SETFAILED:
            return "无法将选定的 SSL 加密引擎设为默认选项。";

        case CURLE_SEND_ERROR:
            return "无法发送网络数据。";

        case CURLE_RECV_ERROR:
            return "接收网络数据失败。";

        case CURLE_SSL_CERTPROBLEM:
            return "本地客户端证书有问题";

        case CURLE_SSL_CIPHER:
            return "无法使用指定的密钥";

        case CURLE_SSL_CACERT:
            return "无法使用已知的 CA 证书验证对等证书";

        case CURLE_BAD_CONTENT_ENCODING:
            return "无法识别传输编码";

        case CURLE_LDAP_INVALID_URL:
            return "LDAP 网址无效";

        case CURLE_FILESIZE_EXCEEDED:
            return "超过了文件大小上限";

        case CURLE_USE_SSL_FAILED:
            return "请求的 FTP SSL 级别失败";

        case CURLE_SEND_FAIL_REWIND:
            return "进行发送操作时，curl 必须回转数据以便重新传输，但回转操作未能成功";

        case CURLE_SSL_ENGINE_INITFAILED:
            return "SSL 引擎初始化失败";

        case CURLE_LOGIN_DENIED:
            return "远程服务器拒绝 curl 登录（7.13.1 新增功能）";

        case CURLE_TFTP_NOTFOUND:
            return "在 TFTP 服务器上找不到文件";

        case CURLE_TFTP_PERM:
            return "在 TFTP 服务器上遇到权限问题";

        case CURLE_REMOTE_DISK_FULL:
            return "服务器磁盘空间不足";

        case CURLE_TFTP_ILLEGAL:
            return "TFTP 操作非法";

        case CURLE_TFTP_UNKNOWNID:
            return "TFTP 传输 ID 未知";

        case CURLE_REMOTE_FILE_EXISTS:
            return "文件已存在，无法覆盖";

        case CURLE_TFTP_NOSUCHUSER:
            return "运行正常的 TFTP 服务器不会返回此错误";

        case CURLE_CONV_FAILED:
            return "字符转换失败";

        case CURLE_CONV_REQD:
            return "调用方必须注册转换回调";

        case CURLE_SSL_CACERT_BADFILE:
            return "读取 SSL CA 证书时遇到问题（可能是路径错误或访问权限问题）";

        case CURLE_REMOTE_FILE_NOT_FOUND:
            return "网址中引用的资源不存在";

        case CURLE_SSH:
            return "SSH 会话中发生无法识别的错误";

        case CURLE_SSL_SHUTDOWN_FAILED:
            return "无法终止 SSL 连接";

        default:
            return "未知的 curl 下载错误";
    }
}

#endif



int main(int argc, char *argv[]) {
    char sService[30], sLog[300], sServerIP[30];
    int read_sock, numfds;
    fd_set readmask;
    struct timeval timeout_select;
    int nError;


    AIcom_ChangeToDaemon();

    // 重定向输出
    //SetTraceFile((char *)"UPGRADE",(char *)CONFIG_FILE);

    /* 与主进程建立联接 */
    char *sMsg = AIcom_GetConfigString((char *) "Config", (char *) "Socket", (char *) CONFIG_FILE);
    if (sMsg == NULL) {
        printf("Fail to get Socket in %s!\n", CONFIG_FILE);
        return (AI_NG);
    };
    strcpy(sService, sMsg);

    server_sock = AIEU_DomainEstablishConnection(sService);
    printf("server_sock=%d\n", server_sock);
    if (server_sock < 0) {
        sprintf(sLog, "UPGRADE Process : AIEU_DomainEstablishConnection %s error!", sService);
        WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
        return AI_NG;
    };

    /**
     * 初始化参数
     */
    memset(&dl_param, 0, sizeof(dl_param));
    dl_param.dl_sock = server_sock;
    dl_param.is_full_pkg_update = False;
    dl_param.status_cond = PTHREAD_COND_INITIALIZER;
    dl_param.status_mutex = PTHREAD_MUTEX_INITIALIZER;

    aa_status = AAW_CTL_DOWNLOAD_INIT;

    // stpcpy(dl_param.save_path,UPGRADE_FULL_PKG_SAVE_PATH);
    // strcat(dl_param.save_path,UPGRADE_FULL_PKG_NAME);
    // 把本进程的标识送给主进程
    PacketHead stHead;
    memset((char *) &stHead, 0, sizeof(PacketHead));
    stHead.iPacketID = PKT_CLIENT_IDENTITY;
    stHead.iRecordNum = UPGRADE_PROCESS_IDENTITY;
    stHead.lPacketSize = sizeof(PacketHead);
    AAWANTSendPacket(server_sock, (char *) &stHead);

    // 初始化本程序中重要的变量

    timeout_select.tv_sec = 10;
    timeout_select.tv_usec = 0;
    for (;;) {
        FD_ZERO(&readmask);
        FD_SET(server_sock, &readmask);
        read_sock = server_sock;

        /* 检查通信端口是否活跃 */
        numfds = select(read_sock + 1, (fd_set *) &readmask, 0, 0, &timeout_select);

        if (numfds <= 0) {
            continue;
        };

        /* 主控程序发来包 */
        if (FD_ISSET(server_sock, &readmask)) {
            char *lpInBuffer = AAWANTGetPacket(server_sock, &nError);
            if (lpInBuffer == NULL) {
                if (nError == EINTR || nError == 0) {  /* 因信号而中断 */
                    printf("signal interrupt\n");
                    continue;
                };
                /* 主进程关闭了联接，本程序也终止！ */
                //  WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"Upgrade Process : Receive disconnect info from Master Process!");
                printf("close sock\n");
                AIEU_TCPClose(server_sock);


            };
            PacketHead *pHead = (PacketHead *) lpInBuffer;
            switch (pHead->iPacketID) {

            #if 0
                case PKT_UPGRADE_CTRL: {

                    E_UPG_CONTROL_UPGRADE_STATUS status=Aawant_Get_Upgrade_Status();
                    if(status==E_UPG_CONTROL_UPGRADE_STATUS_UPGRADING)
                    {
                        continue;
                    } else{
                        Aawant_Set_Upgrade_Status(dl_param.status);
                    }

                    AAWANT_UPG_CTL_STATUS status;

                    TO_UPGRADE_DATA *upData;

                    upData = (TO_UPGRADE_DATA *) (lpInBuffer + sizeof(PacketHead));

                    if (upData->action == DOWNLOAD_START) {
                        printf("Upgrade:start download\n");
                        // Aawant_Set_Upgrade_Status(E_UPG_CONTROL_UPGRADE_STATUS_INITED);
                        status = Aawant_Get_Upgrade_Status();
                        if (status == AAW_CTL_DOWNLOAD_DOING) {
                            printf("It is downloading\n");
                        }
                        if (status == AAW_CTL_UPGRADE_DOING) {
                            printf("It is upgrading\n");
                        } else if (status == AAW_CTL_DOWNLOAD_INIT | status == AAW_CTL_UPGRADE_SUCESS |
                                   status == AAW_CTL_UPGRADE_FAIL
                                   | status == AAW_CTL_DOWNLOAD_CANCEL | status == AAW_CTL_DOWNLOAD_FAIL) {
                            createDownloadPthread();
                        }

                    } else if (upData->action == DOWNLOAD_CONTINUE) {
                        printf("Upgrade:continue\n");
                        // FROM_UPGRADE_DATA upgradeData;

                        // upgradeData.status=0;
                        // upgradeData.code=0;

                        //  AAWANTSendPacket(read_sock,PKT_UPGRADE_CTRL,(char *)&upgradeData, sizeof(upgradeData));
                    } else if (upData->action == DOWNLOAD_PAUSE) {
                        printf("Upgrade:pause\n");

                    } else if (upData->action == DOWNLOAD_CANCEL) {
                        printf("Upgrade:cancel\n");
                        Aawant_Set_Upgrade_Status(&dl_param, AAW_CTL_DOWNLOAD_CANCEL);

                    } else if (upData->action == UPGRADE_START) {
                        printf("=======Upgrade:Get Upgrade Cmd========\n");

                        //Aawant_Set_Upgrade_Status(E_UPG_CONTROL_UPGRADE_STATUS_CANCELLED);

                        dl_param.is_full_pkg_update = True;
                        memset(dl_param.save_path, 0, sizeof(dl_param.save_path));
                        stpcpy(dl_param.save_path, UPGRADE_FULL_PKG_SAVE_PATH);
                        strcat(dl_param.save_path, UPGRADE_FULL_PKG_NAME);

                        int ret = AawantCmd_Flash_ImgData(&dl_param);


                        if (ret == -1) {
                            printf("Upgrade failed\n");
                            Aawant_Set_Upgrade_Status(&dl_param, AAW_CTL_UPGRADE_FAIL);
                            FROM_UPGRADE_DATA upgradeData;
                            upgradeData.status = UPGRADE_FAIL;
                            upgradeData.code = -1;

                            AAWANTSendPacket(server_sock, PKT_UPGRADE_FEEDBACK, (char *) &upgradeData,
                                             sizeof(upgradeData));
                        } else {
                            printf("Upgrade sucess\n");
                            // Aawant_Set_Upgrade_Status(E);
                            FROM_UPGRADE_DATA upgradeData;
                            upgradeData.status = UPGRADE_FINISH_AND_REQUEST_REBOOT;
                            upgradeData.code = 0;

                            AAWANTSendPacket(server_sock, PKT_UPGRADE_FEEDBACK, (char *) &upgradeData,
                                             sizeof(upgradeData));
                        }


                    }


                    break;
            #endif
                case  PKT_GET_SYSTEMTASK_STATUS:
                    break;


                case  PKT_VERSION_UPDATE: {
                    struct UpdateInfoMsg_Iot_Data *updateData;
                    updateData = (struct UpdateInfoMsg_Iot_Data *) (lpInBuffer + sizeof(PacketHead));
                    strcpy(dl_param.url,updateData->updateUrl);
                    strcpy(dl_param.save_path,UPGRADE_FULL_PKG_SAVE_PATH);
                    createDownloadPthread();
                    break;
                }
                case PKT_ROBOT_WIFI_CONNECT: {


                    break;
                }
                case PKT_ROBOT_WIFI_DISCONNECT:{
                    break;
                }

                default:
                    WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"upgraded Process : Receive unknown message from Master Process!");
                    printf("upgraded Process : Receive unknown message from Master Process!\n");

                    break;
            };
            free(lpInBuffer);
        }/* if( FD_ISSET(IOT_sock, &readmask) ) */
    };
}

