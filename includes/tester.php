<?php
/**
 * Created by IntelliJ IDEA.
 * User: ostwald
 * Date: 2/5/19
 * Time: 4:07 PM
 */

include 'doi_importer.inc';
include 'kuali.inc';
include 'wos.inc';

/*
 * Returns DOMDocument
 */
function get_crossref_data () {


    $crossref_xml = file_get_contents('/Users/ostwald/devel/opensky/pubs_to_grants/xml/crossref-sample.xml');

    return $crossref_xml;
}

function get_crossref_doc () {
    $crossref_xml = get_crossref_data();

    $crossref = new DOMDocument("1.0");
    if (!$crossref->loadXML($crossref_xml)) {
        return FALSE;
    }
    return $crossref;
}

function get_mods_as_if_from_crossref() {
    $mods_xml = file_get_contents('/Users/ostwald/devel/opensky/pubs_to_grants/xml/mods_from_crossref.xml');

    $mods = new DOMDocument("1.0");
    if (!$mods->loadXML($mods_xml)) {
        return FALSE;
    }
    return $mods;
}


function kuali_tester() {
//    $award_id = 'FA9550-16-1-0050x';
    $award_id = 'FA9550-16-1-0050';
//    $award_id = '0050';
    $ret = get_kuali_response($award_id);
    echo "\nget_kuali_response returned an ".gettype($ret) . "\n";

    $ret = get_kuali_award_info($award_id);
    if (!$ret) {
        echo ("\naward_info NOT found for $award_id \n");
    } else {
        echo "\nFOUND\n";
        echo json_encode($ret, JSON_PRETTY_PRINT);
    }

}


function translator_post_process_tester () {
    $crossref_xml = get_crossref_doc();
    $mods = get_mods_as_if_from_crossref();
    // post_process($mods, $crossref_xml);


    openskydora_translator_post_processor ($mods, $crossref_xml);
}


function post_process($mods, $crossref_xml) {
    // NOT YET
    return false;
}

function WOI_funder_info_tester() {
    $wos_xml = get_wos_dom (null);

    $funders = array();
    get_wos_funder_info ($wos_xml, $funders);
    // print_r ($funders);
    print_funder_info ($funders);
}

//kuali_tester();
 translator_post_process_tester();