/*
 * Received Management frame processing
 * Copyright (c) 2010, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "common/ieee802_11_defs.h"
#include "common/ieee802_11_common.h"
#include "crypto/aes_wrap.h"
#include "wlantest.h"


static const char * mgmt_stype(u16 stype)
{
	switch (stype) {
	case WLAN_FC_STYPE_ASSOC_REQ:
		return "ASSOC-REQ";
	case WLAN_FC_STYPE_ASSOC_RESP:
		return "ASSOC-RESP";
	case WLAN_FC_STYPE_REASSOC_REQ:
		return "REASSOC-REQ";
	case WLAN_FC_STYPE_REASSOC_RESP:
		return "REASSOC-RESP";
	case WLAN_FC_STYPE_PROBE_REQ:
		return "PROBE-REQ";
	case WLAN_FC_STYPE_PROBE_RESP:
		return "PROBE-RESP";
	case WLAN_FC_STYPE_BEACON:
		return "BEACON";
	case WLAN_FC_STYPE_ATIM:
		return "ATIM";
	case WLAN_FC_STYPE_DISASSOC:
		return "DISASSOC";
	case WLAN_FC_STYPE_AUTH:
		return "AUTH";
	case WLAN_FC_STYPE_DEAUTH:
		return "DEAUTH";
	case WLAN_FC_STYPE_ACTION:
		return "ACTION";
	}
	return "??";
}


static void rx_mgmt_beacon(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct ieee802_11_elems elems;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;
	if (bss->proberesp_seen)
		return; /* do not override with Beacon data */
	bss->capab_info = le_to_host16(mgmt->u.beacon.capab_info);
	if (ieee802_11_parse_elems(mgmt->u.beacon.variable,
				   len - (mgmt->u.beacon.variable - data),
				   &elems, 0) == ParseFailed) {
		if (bss->parse_error_reported)
			return;
		wpa_printf(MSG_INFO, "Invalid IEs in a Beacon frame from "
			   MACSTR, MAC2STR(mgmt->sa));
		bss->parse_error_reported = 1;
		return;
	}

	bss_update(wt, bss, &elems);
}


static void rx_mgmt_probe_resp(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct ieee802_11_elems elems;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;

	bss->capab_info = le_to_host16(mgmt->u.probe_resp.capab_info);
	if (ieee802_11_parse_elems(mgmt->u.probe_resp.variable,
				   len - (mgmt->u.probe_resp.variable - data),
				   &elems, 0) == ParseFailed) {
		if (bss->parse_error_reported)
			return;
		wpa_printf(MSG_INFO, "Invalid IEs in a Probe Response frame "
			   "from " MACSTR, MAC2STR(mgmt->sa));
		bss->parse_error_reported = 1;
		return;
	}

	bss_update(wt, bss, &elems);
}


static void rx_mgmt_auth(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct wlantest_sta *sta;
	u16 alg, trans, status;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;
	if (os_memcmp(mgmt->sa, mgmt->bssid, ETH_ALEN) == 0)
		sta = sta_get(bss, mgmt->da);
	else
		sta = sta_get(bss, mgmt->sa);
	if (sta == NULL)
		return;

	if (len < 24 + 6) {
		wpa_printf(MSG_INFO, "Too short Authentication frame from "
			   MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	alg = le_to_host16(mgmt->u.auth.auth_alg);
	trans = le_to_host16(mgmt->u.auth.auth_transaction);
	status = le_to_host16(mgmt->u.auth.status_code);

	wpa_printf(MSG_DEBUG, "AUTH " MACSTR " -> " MACSTR
		   " (alg=%u trans=%u status=%u)",
		   MAC2STR(mgmt->sa), MAC2STR(mgmt->da), alg, trans, status);

	if (alg == 0 && trans == 2 && status == 0) {
		if (sta->state == STATE1) {
			wpa_printf(MSG_DEBUG, "STA " MACSTR
				   " moved to State 2 with " MACSTR,
				   MAC2STR(sta->addr), MAC2STR(bss->bssid));
			sta->state = STATE2;
		}
	}
}


static void rx_mgmt_deauth(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct wlantest_sta *sta;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;
	if (os_memcmp(mgmt->sa, mgmt->bssid, ETH_ALEN) == 0)
		sta = sta_get(bss, mgmt->da);
	else
		sta = sta_get(bss, mgmt->sa);
	if (sta == NULL)
		return;

	if (len < 24 + 2) {
		wpa_printf(MSG_INFO, "Too short Deauthentication frame from "
			   MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	wpa_printf(MSG_DEBUG, "DEAUTH " MACSTR " -> " MACSTR
		   " (reason=%u)",
		   MAC2STR(mgmt->sa), MAC2STR(mgmt->da),
		   le_to_host16(mgmt->u.deauth.reason_code));

	if (sta->state != STATE1) {
		wpa_printf(MSG_DEBUG, "STA " MACSTR
			   " moved to State 1 with " MACSTR,
			   MAC2STR(sta->addr), MAC2STR(bss->bssid));
		sta->state = STATE1;
	}
}


static void rx_mgmt_assoc_req(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct wlantest_sta *sta;
	struct ieee802_11_elems elems;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;
	sta = sta_get(bss, mgmt->sa);
	if (sta == NULL)
		return;

	if (len < 24 + 4) {
		wpa_printf(MSG_INFO, "Too short Association Request frame "
			   "from " MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	wpa_printf(MSG_DEBUG, "ASSOCREQ " MACSTR " -> " MACSTR
		   " (capab=0x%x listen_int=%u)",
		   MAC2STR(mgmt->sa), MAC2STR(mgmt->da),
		   le_to_host16(mgmt->u.assoc_req.capab_info),
		   le_to_host16(mgmt->u.assoc_req.listen_interval));

	if (ieee802_11_parse_elems(mgmt->u.assoc_req.variable,
				   len - (mgmt->u.assoc_req.variable - data),
				   &elems, 0) == ParseFailed) {
		wpa_printf(MSG_INFO, "Invalid IEs in Association Request "
			   "frame from " MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	sta_update_assoc(sta, &elems);
}


static void rx_mgmt_assoc_resp(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct wlantest_sta *sta;
	u16 capab, status, aid;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;
	sta = sta_get(bss, mgmt->da);
	if (sta == NULL)
		return;

	if (len < 24 + 6) {
		wpa_printf(MSG_INFO, "Too short Association Response frame "
			   "from " MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	capab = le_to_host16(mgmt->u.assoc_resp.capab_info);
	status = le_to_host16(mgmt->u.assoc_resp.status_code);
	aid = le_to_host16(mgmt->u.assoc_resp.aid);

	wpa_printf(MSG_DEBUG, "ASSOCRESP " MACSTR " -> " MACSTR
		   " (capab=0x%x status=%u aid=%u)",
		   MAC2STR(mgmt->sa), MAC2STR(mgmt->da), capab, status,
		   aid & 0x3fff);

	if (status)
		return;

	if ((aid & 0xc000) != 0xc000) {
		wpa_printf(MSG_DEBUG, "Two MSBs of the AID were not set to 1 "
			   "in Association Response from " MACSTR,
			   MAC2STR(mgmt->sa));
	}
	sta->aid = aid & 0xc000;

	if (sta->state < STATE2) {
		wpa_printf(MSG_DEBUG, "STA " MACSTR " was not in State 2 when "
			   "getting associated", MAC2STR(sta->addr));
	}

	if (sta->state < STATE3) {
		wpa_printf(MSG_DEBUG, "STA " MACSTR
			   " moved to State 3 with " MACSTR,
			   MAC2STR(sta->addr), MAC2STR(bss->bssid));
		sta->state = STATE3;
	}
}


static void rx_mgmt_reassoc_req(struct wlantest *wt, const u8 *data,
				size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct wlantest_sta *sta;
	struct ieee802_11_elems elems;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;
	sta = sta_get(bss, mgmt->sa);
	if (sta == NULL)
		return;

	if (len < 24 + 4 + ETH_ALEN) {
		wpa_printf(MSG_INFO, "Too short Reassociation Request frame "
			   "from " MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	wpa_printf(MSG_DEBUG, "REASSOCREQ " MACSTR " -> " MACSTR
		   " (capab=0x%x listen_int=%u current_ap=" MACSTR ")",
		   MAC2STR(mgmt->sa), MAC2STR(mgmt->da),
		   le_to_host16(mgmt->u.reassoc_req.capab_info),
		   le_to_host16(mgmt->u.reassoc_req.listen_interval),
		   MAC2STR(mgmt->u.reassoc_req.current_ap));

	if (ieee802_11_parse_elems(mgmt->u.reassoc_req.variable,
				   len - (mgmt->u.reassoc_req.variable - data),
				   &elems, 0) == ParseFailed) {
		wpa_printf(MSG_INFO, "Invalid IEs in Reassociation Request "
			   "frame from " MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	sta_update_assoc(sta, &elems);
}


static void rx_mgmt_reassoc_resp(struct wlantest *wt, const u8 *data,
				 size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct wlantest_sta *sta;
	u16 capab, status, aid;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;
	sta = sta_get(bss, mgmt->da);
	if (sta == NULL)
		return;

	if (len < 24 + 6) {
		wpa_printf(MSG_INFO, "Too short Reassociation Response frame "
			   "from " MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	capab = le_to_host16(mgmt->u.reassoc_resp.capab_info);
	status = le_to_host16(mgmt->u.reassoc_resp.status_code);
	aid = le_to_host16(mgmt->u.reassoc_resp.aid);

	wpa_printf(MSG_DEBUG, "REASSOCRESP " MACSTR " -> " MACSTR
		   " (capab=0x%x status=%u aid=%u)",
		   MAC2STR(mgmt->sa), MAC2STR(mgmt->da), capab, status,
		   aid & 0x3fff);

	if (status)
		return;

	if ((aid & 0xc000) != 0xc000) {
		wpa_printf(MSG_DEBUG, "Two MSBs of the AID were not set to 1 "
			   "in Reassociation Response from " MACSTR,
			   MAC2STR(mgmt->sa));
	}
	sta->aid = aid & 0xc000;

	if (sta->state < STATE2) {
		wpa_printf(MSG_DEBUG, "STA " MACSTR " was not in State 2 when "
			   "getting associated", MAC2STR(sta->addr));
	}

	if (sta->state < STATE3) {
		wpa_printf(MSG_DEBUG, "STA " MACSTR
			   " moved to State 3 with " MACSTR,
			   MAC2STR(sta->addr), MAC2STR(bss->bssid));
		sta->state = STATE3;
	}
}


static void rx_mgmt_disassoc(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	struct wlantest_bss *bss;
	struct wlantest_sta *sta;

	mgmt = (const struct ieee80211_mgmt *) data;
	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return;
	if (os_memcmp(mgmt->sa, mgmt->bssid, ETH_ALEN) == 0)
		sta = sta_get(bss, mgmt->da);
	else
		sta = sta_get(bss, mgmt->sa);
	if (sta == NULL)
		return;

	if (len < 24 + 2) {
		wpa_printf(MSG_INFO, "Too short Disassociation frame from "
			   MACSTR, MAC2STR(mgmt->sa));
		return;
	}

	wpa_printf(MSG_DEBUG, "DISASSOC " MACSTR " -> " MACSTR
		   " (reason=%u)",
		   MAC2STR(mgmt->sa), MAC2STR(mgmt->da),
		   le_to_host16(mgmt->u.disassoc.reason_code));

	if (sta->state < STATE2) {
		wpa_printf(MSG_DEBUG, "STA " MACSTR " was not in State 2 or 3 "
			   "when getting disassociated", MAC2STR(sta->addr));
	}

	if (sta->state > STATE2) {
		wpa_printf(MSG_DEBUG, "STA " MACSTR
			   " moved to State 2 with " MACSTR,
			   MAC2STR(sta->addr), MAC2STR(bss->bssid));
		sta->state = STATE2;
	}
}


static int check_mmie_mic(const u8 *igtk, const u8 *data, size_t len)
{
	u8 *buf;
	u8 mic[16];
	u16 fc;
	const struct ieee80211_hdr *hdr;

	buf = os_malloc(len + 20 - 24);
	if (buf == NULL)
		return -1;

	/* BIP AAD: FC(masked) A1 A2 A3 */
	hdr = (const struct ieee80211_hdr *) data;
	fc = le_to_host16(hdr->frame_control);
	fc &= ~(WLAN_FC_RETRY | WLAN_FC_PWRMGT | WLAN_FC_MOREDATA);
	WPA_PUT_LE16(buf, fc);
	os_memcpy(buf + 2, hdr->addr1, 3 * ETH_ALEN);

	/* Frame body with MMIE MIC masked to zero */
	os_memcpy(buf + 20, data + 24, len - 24 - 8);
	os_memset(buf + 20 + len - 24 - 8, 0, 8);

	wpa_hexdump(MSG_MSGDUMP, "BIP: AAD|Body(masked)", buf, len + 20 - 24);
	/* MIC = L(AES-128-CMAC(AAD || Frame Body(masked)), 0, 64) */
	if (omac1_aes_128(igtk, buf, len + 20 - 24, mic) < 0) {
		os_free(buf);
		return -1;
	}

	os_free(buf);

	if (os_memcmp(data + len - 8, mic, 8) != 0)
		return -1;

	return 0;
}


static int check_bip(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_mgmt *mgmt;
	u16 fc, stype;
	const u8 *mmie;
	int keyid;
	struct wlantest_bss *bss;

	mgmt = (const struct ieee80211_mgmt *) data;
	fc = le_to_host16(mgmt->frame_control);
	stype = WLAN_FC_GET_STYPE(fc);

	if (stype == WLAN_FC_STYPE_ACTION) {
		if (len < 24 + 1)
			return 0;
		if (mgmt->u.action.category == WLAN_ACTION_PUBLIC)
			return 0; /* Not a robust management frame */
	}

	bss = bss_get(wt, mgmt->bssid);
	if (bss == NULL)
		return 0; /* No key known yet */

	if (len < 24 + 18 || data[len - 18] != WLAN_EID_MMIE ||
	    data[len - 17] != 16) {
		/* No MMIE */
		/* TODO: use MFPC flag in RSN IE instead of IGTK flags */
		if (bss->igtk_set[4] || bss->igtk_set[5]) {
			wpa_printf(MSG_INFO, "Robust group-addressed "
				   "management frame sent without BIP by "
				   MACSTR, MAC2STR(mgmt->sa));
			return -1;
		}
		return 0;
	}

	mmie = data + len - 16;
	keyid = WPA_GET_LE16(mmie);
	if (keyid < 4 || keyid > 5) {
		wpa_printf(MSG_INFO, "Unexpected MMIE KeyID %u from " MACSTR,
			   keyid, MAC2STR(mgmt->sa));
		return 0;
	}
	wpa_printf(MSG_DEBUG, "MMIE KeyID %u", keyid);
	wpa_hexdump(MSG_MSGDUMP, "MMIE IPN", mmie + 2, 6);
	wpa_hexdump(MSG_MSGDUMP, "MMIE MIC", mmie + 8, 8);

	if (!bss->igtk_set[keyid]) {
		wpa_printf(MSG_DEBUG, "No IGTK known to validate BIP frame");
		return 0;
	}

	if (os_memcmp(mmie + 2, bss->ipn[keyid], 6) <= 0) {
		wpa_printf(MSG_INFO, "BIP replay detected: SA=" MACSTR,
			   MAC2STR(mgmt->sa));
		wpa_hexdump(MSG_INFO, "RX IPN", mmie + 2, 6);
		wpa_hexdump(MSG_INFO, "Last RX IPN", bss->ipn[keyid], 6);
	}

	if (check_mmie_mic(bss->igtk[keyid], data, len) < 0) {
		wpa_printf(MSG_INFO, "Invalid MMIE MIC in a frame from "
			   MACSTR, MAC2STR(mgmt->sa));
		return -1;
	}

	wpa_printf(MSG_DEBUG, "Valid MMIE MIC");
	os_memcpy(bss->ipn[keyid], mmie + 2, 6);

	return 0;
}


void rx_mgmt(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_hdr *hdr;
	u16 fc, stype;

	if (len < 24)
		return;

	hdr = (const struct ieee80211_hdr *) data;
	fc = le_to_host16(hdr->frame_control);
	wt->rx_mgmt++;
	stype = WLAN_FC_GET_STYPE(fc);

	if ((hdr->addr1[0] & 0x01) &&
	    (stype == WLAN_FC_STYPE_DEAUTH ||
	     stype == WLAN_FC_STYPE_DISASSOC ||
	     stype == WLAN_FC_STYPE_ACTION))
		check_bip(wt, data, len);

	wpa_printf((stype == WLAN_FC_STYPE_BEACON ||
		    stype == WLAN_FC_STYPE_PROBE_RESP ||
		    stype == WLAN_FC_STYPE_PROBE_REQ) ?
		   MSG_EXCESSIVE : MSG_MSGDUMP,
		   "MGMT %s%s%s DA=" MACSTR " SA=" MACSTR " BSSID=" MACSTR,
		   mgmt_stype(stype),
		   fc & WLAN_FC_PWRMGT ? " PwrMgt" : "",
		   fc & WLAN_FC_ISWEP ? " Prot" : "",
		   MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
		   MAC2STR(hdr->addr3));

	switch (stype) {
	case WLAN_FC_STYPE_BEACON:
		rx_mgmt_beacon(wt, data, len);
		break;
	case WLAN_FC_STYPE_PROBE_RESP:
		rx_mgmt_probe_resp(wt, data, len);
		break;
	case WLAN_FC_STYPE_AUTH:
		rx_mgmt_auth(wt, data, len);
		break;
	case WLAN_FC_STYPE_DEAUTH:
		rx_mgmt_deauth(wt, data, len);
		break;
	case WLAN_FC_STYPE_ASSOC_REQ:
		rx_mgmt_assoc_req(wt, data, len);
		break;
	case WLAN_FC_STYPE_ASSOC_RESP:
		rx_mgmt_assoc_resp(wt, data, len);
		break;
	case WLAN_FC_STYPE_REASSOC_REQ:
		rx_mgmt_reassoc_req(wt, data, len);
		break;
	case WLAN_FC_STYPE_REASSOC_RESP:
		rx_mgmt_reassoc_resp(wt, data, len);
		break;
	case WLAN_FC_STYPE_DISASSOC:
		rx_mgmt_disassoc(wt, data, len);
		break;
	}
}