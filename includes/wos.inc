<?php
/**
 * Created by IntelliJ IDEA.
 * User: ostwald
 * Date: 2/6/19
 * Time: 9:58 AM
 */

/**
 *  * Get WOS response for the given DOI.
 *
 * @param string $id
 *   A DOI.
 *
 * @return XML String containing abstract and funderInfo
 */

function openskydora_doi_get_wos_response($id) {
    // dpm ('getting WOS DATA from web ..');
    $auth_url = "http://search.webofknowledge.com/esti/wokmws/ws/WOKMWSAuthenticate?wsdl";

    try {
        // see http://php.net/manual/en/class.soapclient.php
        $opts = array(
            'ssl' => array(
                'verify_peer'=>false,
                'verify_peer_name'=>false
            )
        );
        // SOAP 1.2 client
        $params = array (
            'login' => "NCAR",
            'password' => "OLCP2YXI",
            'stream_context' => stream_context_create($opts)
        );

        $auth_client = new SoapClient ( $auth_url, $params );

    } catch (Exception $e) {
        error_log ('ERROR: ' . $e->getErrorMessage());
    }

    $auth_response = @$auth_client->authenticate();

    if (!$auth_response) {
        throw new Exception ("Could not authenticate NCAR client for WOS");
    }

    $search_url = "http://search.webofknowledge.com/esti/wokmws/ws/WokSearch?wsdl";
    $search_client = @new SoapClient($search_url);
    $search_client->__setCookie('SID', $auth_response->return);
    $id = '"' . $id . '"';

    $search_array = array(
        'queryParameters' => array(
            'databaseId' => 'WOS',
            'userQuery' => 'DO=' . $id,
            'editions' => array(
                array('collection' => 'WOS', 'edition' => 'SCI')
            ),
            'queryLanguage' => 'en'
        ),
        'retrieveParameters' => array(
            'count' => '5',
            'fields' => array(
                array('name' => 'Date', 'sort' => 'D')
            ),
            'firstRecord' => '1',
            'viewField' => array('collectionName' => 'WOS', 'fieldName' => array ('abstract', 'grants'))
        )
    );

    try {
        $search_response = $search_client->search($search_array);
        if ($search_response->return->recordsFound > 0) {
            $returnedxml = simplexml_load_string($search_response->return->records);
            $returnedxml_str =  $returnedxml->asXML();
        } else {
            $returnedxml_str = NULL;
        }
    } catch (Exception $e) {
        echo $e->getMessage();
        $abstract = "Error";
    }
    return $returnedxml_str;  // return xml string
}


/*
 * Debugging function. Returns XML String
 * $doi param not used for this canned version ....
 */
function openskydora_doi_get_canned_wos_response ($doi) {
//    $path = '/Users/ostwald/tmp/WOS_DATA.xml';
    $path = '/Users/ostwald/devel/opensky/pubs_to_grants/xml/wos_data.xml';
    $wos_data_str = file_get_contents($path);
    return $wos_data_str;
}

/*
 Insert provided $abstract into the $mods DOMDocument
*/
function add_wos_abstract_to_mods ($mods, $abstract) {
    if ($abstract) {
        $abstract_node = $mods->createElement('abstract');
        $abstract_node->nodeValue = $abstract;
        $mods->firstChild->appendChild($abstract_node);
    }
}

/*
 * Returns DOMDocument for provided $doi. Obtains from WOS if there is a doi,
 * otherwise gets canned WOS XML (for debugging).
 */
function openskydora_get_wos_dom ($doi) {
    //    dpm ("openskydora_get_wos_dom ($doi)");
    if ($doi) {
        $wos_xml_source = openskydora_doi_get_wos_response($doi);
    } else {
        $wos_xml_source = openskydora_doi_get_canned_wos_response($doi);
    }

    $wos_xml = new DOMDocument('1.0');
    if (!$wos_xml->loadXML($wos_xml_source)) {
        error_log("WOS load FAILED\n");
        return FALSE;
    }

    return $wos_xml;
}

/*
 * Extract information obtained from WOS webservice response.
 * provided $wos_xml is an XML string
 * - abstract text
 * - award_ids as array
 */
function openskydora_get_wos_data ($wos_xml) {

    $abstract = null;
    $xpath = new DOMXpath($wos_xml);
    $xpath->registerNamespace("ns", $wos_xml->documentElement->namespaceURI);
    $nodes = $xpath->query("//ns:abstract_text/ns:p");

    if (is_null($nodes)) {
        error_log ('Abstract not found in WOS_XML');
    } else {
        $wos_abstract_node = $nodes->item(0);

        if ($wos_abstract_node) {
            $abstract = $wos_abstract_node->nodeValue;
        }
    }


    $award_ids = array();
    $xpath = new DOMXpath($wos_xml);
    $xpath->registerNamespace("ns", $wos_xml->documentElement->namespaceURI);
    $id_nodes = $xpath->query("//ns:fund_ack/ns:grants/ns:grant/ns:grant_ids/ns:grant_id");

    // NOTE: there can be dupe grant_agency names in the data!

    foreach ($id_nodes as $node) {
        $award_ids[] = $node->nodeValue;
    }
    return array (
        'award_ids' => $award_ids,
        'abstract' => $abstract,
    );
}

