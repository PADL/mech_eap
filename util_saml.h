/*
 * Copyright (c) 2010, JANET(UK)
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

#ifndef _UTIL_SAML_H_
#define _UTIL_SAML_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

struct eap_gss_saml_attr_ctx;

OM_uint32
samlDuplicateAttrContext(OM_uint32 *minor,
                         const struct eap_gss_saml_attr_ctx *in,
                         struct eap_gss_saml_attr_ctx **out);

OM_uint32
samlCreateAttrContext(OM_uint32 *minor,
                      gss_cred_id_t acceptorCred,
                      gss_name_t initiatorName,
                      gss_buffer_t buffer,
                      struct eap_gss_saml_attr_ctx **pCtx);

OM_uint32
samlReleaseAttrContext(OM_uint32 *minor,
                       struct eap_gss_saml_attr_ctx **ctx);

OM_uint32
samlGetAttributeTypes(OM_uint32 *minor,
                      const struct eap_gss_saml_attr_ctx *ctx,
                      void *data,
                      OM_uint32 (*addAttribute)(OM_uint32 *, void *, gss_buffer_t));

OM_uint32
samlGetAttribute(OM_uint32 *minor,
                 struct eap_gss_saml_attr_ctx *ctx,
                 gss_buffer_t attr,
                 int *authenticated,
                 int *complete,
                 gss_buffer_t value,
                 gss_buffer_t display_value,
                 int *more);

OM_uint32
samlSetAttribute(OM_uint32 *minor,
                 struct eap_gss_saml_attr_ctx *ctx,
                 int complete,
                 gss_buffer_t attr,
                 gss_buffer_t value);

OM_uint32
samlDeleteAttribute(OM_uint32 *minor,
                    struct eap_gss_saml_attr_ctx *ctx,
                    gss_buffer_t attr);

OM_uint32
samlExportAttrContext(OM_uint32 *minor,
                      struct eap_gss_saml_attr_ctx *ctx,
                      gss_buffer_t buffer);

OM_uint32
samlImportAttrContext(OM_uint32 *minor,
                      gss_buffer_t buffer,
                      struct eap_gss_saml_attr_ctx **pCtx);

OM_uint32
samlGetAssertion(OM_uint32 *minor,
                 struct eap_gss_saml_attr_ctx *ctx,
                 gss_buffer_t assertion);
 

OM_uint32
samlMapNametoAny(OM_uint32 *minor,
                 const struct eap_gss_saml_attr_ctx *ctx,
                 int authenticated,
                 gss_buffer_t type_id,
                 gss_any_t *output);

OM_uint32
samlReleaseAnyNameMapping(OM_uint32 *minor,
                          const struct eap_gss_saml_attr_ctx *ctx,
                          gss_buffer_t type_id,
                          gss_any_t *input);

#ifdef __cplusplus
}
#endif

#endif /* _UTIL_SAML_H_ */