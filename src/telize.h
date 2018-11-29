#ifndef __H_TELIZE_H
#define __H_TELIZE_H

#include <kore/kore.h>
#include <kore/http.h>

#include <maxminddb.h>

#define ENTRY_TYPE_UINT32	0
#define ENTRY_TYPE_STRING	1
#define ENTRY_TYPE_DOUBLE	2

int	request_ip(struct http_request *);
int	request_json_ip(struct http_request *);
int	request_location(struct http_request *);

int	telize_init(int);
int	telize_request_ip(struct http_request *, char *, size_t);
void	telize_getdata(struct kore_buf *, MMDB_lookup_result_s *,
	    MMDB_entry_data_s *, char *, int, ...);

int	location(struct http_request *);

extern MMDB_s telize_asn;
extern MMDB_s telize_city;

#endif
