/*
 * gsseap_err.c:
 * This file is automatically generated; please do not edit it.
 */

#include <stdlib.h>

#define N_(a) a

static const char * const text[] = {
	N_(""),
	N_("Buffer is incorrect size"),
	N_("Mechanism OID is incorrect"),
	N_("Token header is malformed or corrupt"),
	N_("Token is missing data"),
	N_("Packet was replayed in wrong direction"),
	N_("Received token ID does not match expected token ID"),
	N_("Critical inner token type unavailable"),
	N_("Missing required inner token"),
	N_("Duplicate inner token received"),
	N_("Recieved invalid inner token for current state"),
	N_("EAP key unavailable"),
	N_("EAP key too short"),
	N_("Authentication rejected by RADIUS server"),
	N_("Received unknown response code from RADIUS server"),
	N_("RADIUS response is missing EAP request"),
	N_("Generic RADIUS failure"),
	N_("Proxy had no route to identity provider realm"),
	N_("IDP Administratively Prohibits Request"),
	N_("Context is already fully established"),
	N_("Attempt to use incomplete security context"),
	N_("Context token is malformed or corrupt"),
	N_("Error token is malformed or corrupt"),
	N_("Bad context option"),
	N_("Name is not a valid service name"),
	N_("Initiator identity must be a valid name"),
	N_("Could not determine local host name"),
	N_("Could not determine acceptor identity"),
	N_("Acceptor identity different than expected"),
	N_("Acceptor name is too long or has too many components"),
	N_("Name token is malformed or corrupt"),
	N_("Unable to map name to a local identity"),
	N_("Credential usage type is unknown"),
	N_("Credential usage does not match requested usage"),
	N_("Credential is not usable with this mechanism"),
	N_("Attributes indicate credentials have expired"),
	N_("Bad credential option"),
	N_("Default credentials identity unavailable"),
	N_("Missing default password or other credentials"),
	N_("Credential is already fully resolved"),
	N_("CA Certificate blob could not be parsed"),
	N_("Unable to start identity service"),
	N_("No identity selected"),
	N_("Identity service installation error"),
	N_("Identity service OS error"),
	N_("Identity service IPC error"),
	N_("Unknown identity service error"),
	N_("Bad RFC 4121 wrap or MIC token"),
	N_("IOV is missing required buffer"),
	N_("Stream IOV can only contain a single data buffer"),
	N_("Padding IOV is not permitted for RFC 4121 tokens"),
	N_("Unknown quality of protection specified"),
	N_("PRF input too long"),
	N_("PRF key usage type is unknown"),
	N_("Failed to initialize EAP library"),
	N_("Failed to create EAP state machine"),
	N_("Failed to step EAP state machine"),
	N_("EAP peer authentication failure"),
	N_("Received bad EAP message"),
	N_("Failed to initialize RadSec library"),
	N_("Failed to create RadSec context"),
	N_("Name has no attributes"),
	N_("Failed to initialize attribute providers"),
	N_("Unknown naming attribute"),
	N_("Serialised attributes are malformed or corrupt"),
	N_("Failed to initialize attribute context"),
	N_("Failed to initialize SAML library"),
	N_("Failed to process SAML security policy"),
	N_("Failed in SAML binding processing"),
	N_("Failed to process SAML profile"),
	N_("Non-recoverable failure in SAML profile processing"),
	N_("Temporary failure in SAML profile processing"),
	N_("Failure related to SAML metadata use"),
	N_("Failed to initialize Shibboleth"),
	N_("Failure during local attribute processing"),
	N_("Failed to extract local attributes"),
	N_("Failed to filter local attributes"),
	N_("Failed to resolve local attributes"),
	N_("Local attribute configuration failure"),
	N_("Failed to communicate with local attribute server"),
	N_("Channel bindings do not match"),
	N_("Channel binding token missing"),
	N_("Could not find symbol in mechanism glue"),
	N_("Bad mechanism invoke OID"),
    0
};

struct error_table {
    char const * const * msgs;
    long base;
    int n_msgs;
};
struct et_list {
    struct et_list *next;
    const struct error_table * table;
};
extern struct et_list *_et_list;

const struct error_table et_eapg_error_table = { text, 2109382912L, 84 };

static struct et_list link = { 0, 0 };

void initialize_eapg_error_table_r(struct et_list **list);
void initialize_eapg_error_table(void);

void initialize_eapg_error_table(void) {
    initialize_eapg_error_table_r(&_et_list);
}

/* For Heimdal compatibility */
void initialize_eapg_error_table_r(struct et_list **list)
{
    struct et_list *et, **end;

    for (end = list, et = *list; et; end = &et->next, et = et->next)
        if (et->table->msgs == text)
            return;
    et = malloc(sizeof(struct et_list));
    if (et == 0) {
        if (!link.table)
            et = &link;
        else
            return;
    }
    et->table = &et_eapg_error_table;
    et->next = 0;
    *end = et;
}
