<?php
/**
 * Created by IntelliJ IDEA.
 * User: ostwald
 * Date: 2/6/19
 * Time: 9:46 AM
 *
 *  NOTE! This file should be identical to openskydora module kuali_api.inc
 *
 */

/**
 * Remove "special chars" (see regex) from given string
 * @param $raw
 * @return mixed
 */
function sanitize_id ($raw) {
    return preg_replace("/[^A-Za-z0-9 ]/", '', $raw);
}

/**
 * get the number stored in Kuali for a match
 *
 * @param $award_id
 * @param $match_criterion
 * @return the kuali award_id that was matched, or null if there is no match
 */
function get_kuali_award_id ($award_id, $match_criterion="SMART_PARTIAL") {
    $info = get_kuali_award_info ($award_id, $match_criterion);
    if (!$info) {
        return null;
    }

    $fields_to_check = array ('sponsorAwardId', 'fainId');
    forEach ($fields_to_check as $field) {
        if (endsWith(sanitize_id($info[$field]), sanitize_id($award_id))) {
            return $info[$field];
        }
    }
    return null;
}

/*
 * Returns object associated with provided award_id if
 * there is a single award returned by kuali API.
 * othewise, returns null.
 *
 * NOTE: kuali may return multiple items if there are multiple UCAR PI's for
 * a given grant. Therefore, we have to filter results for unique sponsorAwardId
 */
function get_kuali_award_info ($award_id, $match_criterion="SMART_PARTIAL") {
    $kuali_resp = get_kuali_response($award_id);
    return parse_kuali_award_info ($award_id, $kuali_resp, $match_criterion);
}

/**
 * legacy award_ids, as well as ids in the no_title_list, should
 * display only an id (not a grant title)
 * - legacies will not be found in kuali
 * - no_titles WILL be found but we don't want to show the title cause
 * it isn't reliable
 */
function get_openskydora_kuali_award_info ($award_id, $match_criterion="SMART_PARTIAL") {
    $key = 'kuali_award_info_'.$award_id;
    if ($cache = @cache_get($key)) {
        $info = $cache->data;
    }
    else {
        $kuali_resp = get_kuali_response($award_id);
        $info = parse_kuali_award_info ($award_id, $kuali_resp, $match_criterion);
        cache_set ($key, $info);
    }
    return $info;
    
}

/*
 * Returns single Kuali API result for provided award_id if possible, null otherwise.
 *
 * kuali_resp is response from API, which may include several candidates (Kuali
 * API matches against 5 righTmost characters of award_id). This function must
 * choose the best match if there more than 1 candidates.
 */
function parse_kuali_award_info ($award_id, $kuali_resp, $match_criterion="SMART_PARTIAL") {

    if (!isset ($kuali_resp[0])) {
        // this is an object, which means an empty result
        return null;
    }

    // we're dealing with an array
    $len_results = count($kuali_resp);

    if ($len_results == 0) {
        return null;
    }

    // return first result with matchType == full (test for STRICT first)
    foreach ($kuali_resp as $result) {
        if ($result['matchType'] == 'full') {
            return $result;
        }
    }

    if ($match_criterion == 'STRICT') {
        return null;
    }

    // if criteria is NOT STRICT, return first result that meets SMART_PARTIAL criteria
    foreach ($kuali_resp as $result) {

        // SMART_PARTIAL: try to match all non-special query chars with sponsorAwardId

        // look in sponsorAwardId and fainId for partial patch

        $fields_to_check = array ('sponsorAwardId', 'fainId');
        foreach($fields_to_check as $field) {

            // normalize IDS (to remove any "special characters"
            // NOTE: do we want to remove spaces??
            $kuali_award_id = sanitize_id($result[$field]);
            $sought_award_id = sanitize_id ($award_id);

            // if $kuali_award_id is shorter than $sought_award_id, shorten $sought_award_id
            // to match length (but shorten from left)
            $extra = strlen($sought_award_id) - strlen($kuali_award_id);
            if ($extra > 0) {
                $sought_award_id = substr($sought_award_id, $extra);

            }

            if (endsWith($kuali_award_id, $sought_award_id)) {
                // watchdog('kuali', "$award_id - partial match with $field field");
                return $result;
            }
        }

    }

    // there are no strict matches and no SMART_PARTIAL matches
    // if the match_criteria is NAIVE_PARTIAL just return the first result
    // (if there is one) since all results are NAIVE_PARTIAL
    if ($match_criterion == 'NAIVE_PARTIAL') {
        return $kuali_resp[0];
    }

    return null;
}

/*
 * returns kauli response for proviced $award_id as object or array
 */
function get_kuali_response($award_id) {

    if (variable_get('islandora_repository_pid') == 'islandora:root') {
        $skip_award_ids = array();
    } else {
        $skip_award_ids = array(
              '01830',
              '99999',
        );
    }

    // we do not provide kuali info for skipped award ids                                                                    
    if (in_array (strtoupper($award_id), $skip_award_ids)) {
        return null;
    }

    $base_url = 'https://www.fin.ucar.edu/kualiapi/awardsbysponsorawardid';
    $username = 'openskykcapi';
    $password = 'W!n+er5now#';

    $ch = curl_init();
    $options = array(
        CURLOPT_URL => $base_url.'?sponsorAwardId='.$award_id,
        CURLOPT_USERPWD => $username . ":" . $password,
        CURLOPT_RETURNTRANSFER => 1
    );

    curl_setopt_array($ch, $options);
    $resp = curl_exec($ch);
    curl_close($ch);

    $response = json_decode($resp, true);

   return $response;

}


