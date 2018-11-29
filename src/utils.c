#include "telize.h"

MMDB_s telize_asn;
MMDB_s telize_city;

int
telize_init(int state)
{
	if (state != KORE_MODULE_LOAD) {
		kore_log(LOG_NOTICE, "denying reload of module, not fixed");
		return (KORE_RESULT_ERROR);
	}

	if (MMDB_open("/var/db/GeoIP/GeoLite2-City.mmdb",
	    MMDB_MODE_MMAP, &telize_city) != MMDB_SUCCESS)
		fatalx("can't open GeoLite2 City database: %s", errno_s);

	if (MMDB_open("/var/db/GeoIP/GeoLite2-ASN.mmdb",
	    MMDB_MODE_MMAP, &telize_asn) != MMDB_SUCCESS)
		fatalx("can't open GeoLite2 ASN database: %s", errno_s);

	return (KORE_RESULT_OK);
}

void
telize_getdata(struct kore_buf *json, MMDB_lookup_result_s *lookup,
    MMDB_entry_data_s *entry_data, char *field, int type, ...)
{
	va_list keys;
	va_start(keys, type);

	MMDB_vget_value(&lookup->entry, entry_data, keys);

	if (entry_data->has_data) {
		switch(type) {
		case ENTRY_TYPE_UINT32:
			kore_buf_appendf(json, ",\"%s\":%d",
			    field, entry_data->uint32);
			break;
		case ENTRY_TYPE_STRING:
			kore_buf_appendf(json, ",\"%s\":\"%.*s\"",
			    field, entry_data->data_size,
			    entry_data->utf8_string);
			break;
		case ENTRY_TYPE_DOUBLE:
			kore_buf_appendf(json, ",\"%s\":%.4f",
			    field, entry_data->double_value);
			break;
		}
	}

	va_end(keys);
}

int
telize_request_ip(struct http_request *req, char *buf, size_t len)
{
	char		*ptr;
	void		*addr;
	size_t		hdr_len;
	const char	*hdr_ip;

	switch (req->owner->family) {
	case AF_INET:
		addr = &req->owner->addr.ipv4.sin_addr;
		break;
	case AF_INET6:
		addr = &req->owner->addr.ipv6.sin6_addr;
		break;
	default:
		kore_log(LOG_ERR, "unsupported family %d", req->owner->family);
		return (KORE_RESULT_ERROR);
	}

	if (inet_ntop(req->owner->family, addr, buf, len) == NULL) {
		kore_log(LOG_ERR, "inet_ntop failed (%s)", errno_s);
		return (KORE_RESULT_ERROR);
	}

	if (!http_request_header(req, "x-forwarded-for", &hdr_ip))
		return (KORE_RESULT_OK);

	if ((ptr = strchr(hdr_ip, ',')) != NULL) {
		hdr_len = ptr - hdr_ip;
	} else {
		hdr_len = strlen(hdr_ip);
	}

	if (hdr_len > len - 1) {
		kore_log(LOG_ERR, "IP in X-Forwarded-For too large");
		return (KORE_RESULT_ERROR);
	}

	memcpy(buf, hdr_ip, hdr_len);
	buf[hdr_len] = '\0';

	if (inet_pton(AF_INET, buf, &req->owner->addr) == -1) {
		if (inet_pton(AF_INET6, buf, &req->owner->addr) == -1) {
			kore_log(LOG_ERR, "Malformed IP in X-Forwarded-For");
			return (KORE_RESULT_ERROR);
		}
	}

	return (KORE_RESULT_OK);
}
