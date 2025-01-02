#ifndef __FILE_TRANS_H__
#define __FILE_TRANS_H__



#define FILE_TRANS_BUFFER_SIZE 8192
int handle_file_trans_request(char *response, int response_max_len, const char *request_data, int request_len);

#endif  