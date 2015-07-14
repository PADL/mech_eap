/*
 * Copyright (c) 2011, JANET(UK)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of JANET(UK) nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Return context handle properties.
 */

#include "gssapiP_eap.h"

OM_uint32 GSSAPI_CALLCONV
gss_inquire_context(OM_uint32 *minor,
                    gss_ctx_id_t ctx,
                    gss_name_t *src_name,
                    gss_name_t *targ_name,
                    OM_uint32 *lifetime_rec,
                    gss_OID *mech_type,
                    OM_uint32 *ctx_flags,
                    int *locally_initiated,
                    int *open)
{
    OM_uint32 major, tmpMinor;

    if (ctx == GSS_C_NO_CONTEXT) {
        *minor = EINVAL;
        return GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT;
    }

    GSSEAP_MUTEX_LOCK(&ctx->mutex);

    if (src_name != NULL) {
        if (ctx->initiatorName != GSS_C_NO_NAME) {
            major = gssEapDuplicateName(minor, ctx->initiatorName, src_name);
            if (GSS_ERROR(major))
                goto cleanup;
        } else
            *src_name = GSS_C_NO_NAME;
    }

    if (targ_name != NULL) {
        if (ctx->acceptorName != GSS_C_NO_NAME) {
            major = gssEapDuplicateName(minor, ctx->acceptorName, targ_name);
            if (GSS_ERROR(major))
                goto cleanup;
        } else
            *targ_name = GSS_C_NO_NAME;
    }

    if (lifetime_rec != NULL)
        gssEapContextTime(&tmpMinor, ctx, lifetime_rec);

    if (mech_type != NULL) {
        major = gssEapCanonicalizeOid(minor, ctx->mechanismUsed, 0, mech_type);
        if (GSS_ERROR(major))
            goto cleanup;
    }

    if (ctx_flags != NULL) {
        *ctx_flags = ctx->gssFlags;
    }

    if (locally_initiated != NULL) {
        *locally_initiated = CTX_IS_INITIATOR(ctx);
    }

    if (open != NULL) {
        *open = CTX_IS_ESTABLISHED(ctx);
    }

    major = GSS_S_COMPLETE;
    *minor = 0;

cleanup:
    GSSEAP_MUTEX_UNLOCK(&ctx->mutex);

    if (GSS_ERROR(major)) {
        gssEapReleaseName(&tmpMinor, src_name);
        gssEapReleaseName(&tmpMinor, targ_name);
    }

    return major;
}
