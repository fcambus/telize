#include <kore/kore.h>
#include <kore/http.h>

int		ip(struct http_request *);

int
ip(struct http_request *req)
{
	http_response(req, 200, NULL, 0);
	return (KORE_RESULT_OK);
}
