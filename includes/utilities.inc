<?php
/**
 * Assumes PID is last segment of request path
 * e.g., https://osstage2cl.dls.ucar.edu/islandora/object/technotes:546
 */
function openskydora_get_pid_from_request () {

    // we used to restrict to object pages, but now we'd like this to
    // also work with embargo pages (openskydora/embargo)
    /*
    $pat = 'islandora/object';

    if (substr (request_path(), 0, strlen($pat)) != $pat) {
        return null;
    }
    */

    $path_parts = explode('/', request_path());
    $tail = array_pop($path_parts);
    $parts = explode (':', $tail);
    if (count($parts) == 2) {
        return $tail;
    }
    return null;
}

function openskydora_get_search_term_from_request() {
    $pat = ISLANDORA_SOLR_SEARCH_PATH;

    $path = request_path();

    if (strlen($path) <= strlen($pat) || substr (request_path(), 0, strlen($pat)) != $pat) {
        return null;
    }

    $parts = explode('/',request_path());
    $term = array_pop($parts);
    return trim($term);
}

function openskydora_truncate($str) {
    $truncated = truncate_utf8($str, 50, $wordsafe=TRUE);
    if (strlen ($str) != strlen($truncated)) {
        $truncated = "$truncated ...";
    }
    return $truncated;
}

/** 
 * Get the fedora object for provided pid.
 */
function openskydora_get_fedora_object($pid) {
    $my_islandora_tuque = new IslandoraTuque();
    $repository = $my_islandora_tuque->repository;
    try {
        return $repository->getObject($pid);
    } catch (Exception $e) {
        // dsm ('object ('.$pid.') not in repository');
        return null;
    }
}

/**
 * return array representing DC metadata for provided $pid, or null if not found 
 */
function openskydora_get_dc_array ($pid) {
    $fedora_object = openskydora_get_fedora_object($pid);
    if ($fedora_object == null) {
        // dsm ('object ('.$pid.') not in repository');
        return null;
    }

    $dc = $fedora_object['DC'];
    $dc_object = DublinCore::importFromXMLString($dc->content);
    $dc_array = $dc_object->asArray();
    // $title = $dc_array['dc:title']['value'];
    return $dc_array;
}


/**
 *
 */
function openskydora_get_pid_subcollections_pids ($parent_pid) {

    $key = 'subcollections_pids_'.$parent_pid;
    if ($cache = @cache_get($key)) {
        $pids = $cache->data;
    }
    else {
        $obj = openskydora_get_fedora_object($parent_pid);
        if ($obj != null) {
            $pids =  openskydora_get_obj_subcollection_pids($obj);
        } else {
            $pids = array();
        }
        cache_set ($key, $pids);
    }
    return $pids;
}

/**
 * Gets a list of ALL subcollections under parent collection (by recursively calling 
 * $openskydora_get_subcollections.
 *
 * @param string $parent_pid
 *   The parent collection object.
 *
 * @return array
 *   The list of child PIDs.
 */

function openskydora_get_subcollections_recursive($parent_pid){

    $key = 'subcollections_recursive_'.$parent_pid;

    if ($cache = @cache_get($key)) {
        $subs = $cache->data;
    } else {
        $subs = array($parent_pid);
        $children = openskydora_get_pid_subcollections_pids($parent_pid);
        if (count($children)) {
            foreach ($children as $child) {
                $subs = array_merge($subs, openskydora_get_subcollections_recursive($child));
            }
        }
        cache_set($key, $subs);
    }
    return $subs;
}

/**
 * Gets a list of all immediate child PIDs.
 *
 * @param AbstractObject $object
 *   The parent collection object.
 *
 * @return array
 *   The list of child PIDs.
 */
function openskydora_get_obj_subcollection_pids(AbstractObject $object) {
    $get_pid = function($o) {
        return $o['pid'];
    };
    return array_map($get_pid, openskydora_get_obj_subcollection_infos ($object));
}
/**
   returns true if the given pid identifies an OpenSky item
 */
function openskydora_is_item_pid($pid) {
    $suffix = explode(':', $pid)[1];
    return $suffix != '*' && is_numeric($suffix);
}

/**
   returns true if the given pid identifies an OpenSky collection
 */

function openskydora_is_collection_pid ($pid) {
    $suffix = explode(':', $pid)[1];
    return $suffix != '*' && !is_numeric($suffix);
}

/**
   returns true if the given pid is a collection and the collection
   contains at least one subcollection
 */
function openskydora_has_subcollections ($pid) {
    return (count(openskydora_get_pid_subcollections_pids($pid)) > 0);
}

/**
 * Returns list of openskydora_collection_info objects:
 * - pid
 * - title
 */
/**
 * Gets a list of all immediate child PIDs.
 *
 * @param AbstractObject $object
 *   The parent collection object.
 *
 * @return array
 *   The list of openskydora_collection_info objects:
 *    - pid
 *    - title
 */
function openskydora_get_obj_subcollection_infos(AbstractObject $object) {
    module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');

    $params = array(
        'object' => $object,
        'page_size' => -1,
        'model' => "<info:fedora/islandora:collectionCModel>",
    );

    $map_results = function($o) {
        return array (
            'pid' => $o['object']['value'],
            'uri' => $o['object']['uri'],
            'title' => $o['title']['value'],
        );
    };
    $query_info = islandora_basic_collection_get_query_info($params);
    $results = $object->repository->ri->query($query_info['query'], $query_info['type']);
    return array_map($map_results, $results);
}

function openskydora_get_pid_subcollection_infos ($parent_pid) {

    if (!$parent_pid) {
        return array ();
    }
    $key = 'subollection_infos_'.$parent_pid;

    if ($cached = @cache_get($key)) {
        $infos = $cached->data;
    } else {
        $obj = openskydora_get_fedora_object($parent_pid);
        if ($obj != null) {
            $infos = openskydora_get_obj_subcollection_infos($obj);
        }
        else {
            $infos = array();
        }
        cache_set ($key, $infos);
    }

    return $infos;
}


/**
 * Create a mapping from each collection to the (non-recursive) subcollections under it.
 * Currently only used on scratch (demo) page but could be used to cache the tree
 * structure for faster lookup.
 * @return array
 */
function openskydora_get_collection_tree () {
    $base_pid = variable_get('islandora_repository_pid');
    $tree = array();
    foreach (openskydora_get_subcollections_recursive($base_pid) as $pid) {
        $tree[$pid] = openskydora_get_pid_subcollections_pids($pid);
    }
    return $tree;
}

/**
 * GEt list of searchable subcollections, which are those subcollections (recursively)
 * that contain items (and not subcollections)
 *
 * @param $pid
 * @return array
 */
function openskydora_get_searchable_subcollections ($pid) {
    $descendants = openskydora_get_subcollections_recursive($pid);
    $tree = array();
    foreach($descendants as $child) {
        if (!openskydora_has_subcollections($child)) {
            $tree[] = $child;
        }
    }
    return $tree;
}

/*
View this collection (identified by $pid) in search view.
Called in islandora_sulution_pack_collection/theme/theme.inc to redirect
collections to search view
*/
function openskydora_redirect_to_search_view ($pid) {
    $options = array('query' => array(
        'type'=>'dismax',
        'collection'=>$pid
    ));
    drupal_goto('/islandora/search/',$options);
}
/**
 * IMplements hook_islandora_solr_query
 *
 * HARD-CODE fields returned by solr query to make solr responses smaller                                                           
 * and thus avoid PHP Out of memory errors 
 */
function openskydora_islandora_solr_query($islandora_solr_query) {

    $islandora_solr_query->solrParams['fl'] = 'PID,mods*,fgs*,dc.*,dsmd_DC*,keyDate*,RELS_EXT*,*Display*,fedora_datastreams_ms';
}

/**
 * $qp is an instance of IslandoraSolrQueryProcessor
 * if there is a collection parameter, add filters to limit search to named
 * collection and it's subcollections if any exist
 */
function openskydora_islandora_solr_query_alter($qp) {

    // if sort is not set for search pages, then set it to keyDate desc
    if (current_path() == ISLANDORA_SOLR_SEARCH_PATH  && !isset($qp->solrParams['sort'])) {
        $qp->solrParams['sort'] = 'keyDate desc';
    }

    if (isset($qp->internalSolrParams['collection'])) {
        $collection = $qp->internalSolrParams['collection'];

        // create $fq_new by removing all RELS_EXT_isMemberOfCollection_uri_ms
        // clauses
        $fq_new = array();
        $pat = 'RELS_EXT_isMemberOfCollection_uri_ms';
        if (isset($qp->solrParams['fq'])) {
            $fq_old = $qp->solrParams['fq'];
            unset($qp->solrParams['fq']);
            foreach ($fq_old as $item) {
                if (substr($item, 0, strlen($pat)) !== $pat) {
                    $fq_new[] = $item;
                }
            }
        }

        // construct a filter to search all searchable sub-collections (those
        // that have no sub-collections).
        $searchable_collections = openskydora_get_searchable_subcollections($collection);

        $children_params = array();
        foreach ($searchable_collections as $searchable) {
            $children_params[] = 'RELS_EXT_isMemberOfCollection_uri_ms:"info:fedora/'.$searchable.'"';
        }

        $searchable_collection_clause = implode(' OR ', $children_params);

        $qp->solrParams['fq'] = array_merge($fq_new, array($searchable_collection_clause));
    }
}

/**
 * implements hook_islandora_solr_results_alter
 *
 * Set thumbnail for citationCModel objects to OpenSky custom image
 * (BiblioCitOnly.png);
 */
function openskydora_islandora_solr_results_alter(&$object_results, $query_processor) {

    $default_path = drupal_get_path('module', 'islandora_solr') . '/images/defaultimg.png';
    foreach ($object_results as $object_index => $object_result) {
        if (isset ($object_results[$object_index]['thumbnail_url']) && $object_results[$object_index]['thumbnail_url'] == $default_path &&
            isset ($object_results[$object_index]['content_models'])) {
            if (strpos($object_results[$object_index]['content_models'][0], 'citationCModel') !== false) {
                $object_results[$object_index]['thumbnail_url'] = drupal_get_path('module', 'openskydora') . '/images/BiblioCitOnly.png';
            }
        }
    }
}

/**
 * Supply in collections info for islandora_scholar (citation model).
 * Called from islandora_scholar_get_metadata_display
 *    (in islandora_scholar/includes/utilities.inc)
 */
function openskydora_get_in_collections_display ($object) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    $parent_collections = islandora_get_parents_from_rels_ext($object);
    return theme('openskydora_in_collections', array(
        'parent_collections' => $parent_collections));
}

/**
 * Grab the DOI if there is one, and return markup for displaying it
 */
function openskydora_retrieve_doi_markup(AbstractObject $object) {
    $markup = '';
    $mods_xml = simplexml_load_string($object['MODS']->content);
    $mods_xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
    $doi_xpath = $mods_xml->xpath('//mods:identifier[@type="doi"]');
    if ($doi_xpath) {
        $doi_link = (string) $doi_xpath[0];
        $markup = '<div class="openskydora-info">Published Version: <a href="http://dx.doi.org/' . $doi_link . '">http://dx.doi.org/' . $doi_link . '</a> </div>';
    }
    return $markup;
}

/**
 * This alter hook gets called but I don't understand what the heck
 * its good for
 */
function openskydora_islandora_metadata_object_description_alter(AbstractObject $object) {
    // dsm ('openskydora_islandora_metadata_object_description_alter');
    // dsm($object);
}


/**
 * this function is poorly named! It only grabs the items that have an
 * authorized genre attribute. There be other related items in the object

 * related items returned by this function have the following fields:
 * - title
 * - publisher
 * - genre
 * - doi
 * - #markup
 */
function openskydora_retrieve_related_items(AbstractObject $object) {
    $related_items = array();
    $mods_xml = simplexml_load_string($object['MODS']->content);
    $mods_xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
    $related_item_xpath = $mods_xml->xpath('//mods:relatedItem[mods:genre[@authorityURI="https://doi.org/10.5438/0014"]]');
    if ($related_item_xpath) {
        foreach ($related_item_xpath as $related_item_el) {
            $item = array ();
            $related_item_el->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
            $title_path = $related_item_el->xpath('mods:titleInfo/mods:title');
            if ($title_path) {
                $item['title'] = $title_path[0]->__toString();
            }

            $publisher_path = $related_item_el->xpath('mods:originInfo/mods:publisher');
            if ($publisher_path) {
                $item['publisher'] = $publisher_path[0]->__toString();
            }

            $genre_path = $related_item_el->xpath('mods:genre');
            if ($genre_path) {
                $item['genre'] = $genre_path[0]->__toString();
            }

            $doi_path = $related_item_el->xpath('mods:identifier[@type="DOI"]');
            if ($doi_path) {
                $item['doi'] = $doi_path[0]->__toString();
            }
            $related_items[] = $item;
        }
    }
    return $related_items;
}
/**
 * related_items have the fields:
 * - title
 * - publisher
 * - genre ('Dataset' | 'Software' | 'other')
 * - doi
 * - #markup

 * RETURNS : 2-d array (keys = software, datasets, other) holding
 * lists of related items
 */
function openskydora_get_related_items(AbstractObject $object) {
    $all_items = openskydora_retrieve_related_items($object);
    
    $output = array(
            'software' => array(),
            'datasets' => array(),
            'other' => array(),
        );
    foreach ($all_items as &$item) {
        $genre = $item['genre'];
        $doi_url = 'https://doi.org/' . @$item['doi'];
        $item['#markup'] = '<li><a href="' . $doi_url . '">' . $item['title'] . '</a></li>';

        // normalize gender to 'software', 'dataset' or 'other'
        if ($genre == 'Software') {
            $normalized_genre = 'software';
        }
        else if ($genre == 'Dataset') {
            $normalized_genre = 'datasets';
        } else {
            $normalized_genre = 'other';
        }
        $output[$normalized_genre][] = $item;

    }
    
    return $output;
}

/**
 * funding info is obtained from kuail (using the award_id found in
 * MODS)
 * 
 * funding info items have these fields:
 * - 
 */
function openskydora_get_funding_info(AbstractObject $object) {
    $mods_xml = simplexml_load_string($object['MODS']->content);
    $mods_xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
    $award_info_nodes = $mods_xml->xpath('//mods:note[@type="funding"]');

    $funders = array();
    
    foreach($award_info_nodes as $award_info_node) {
        $award_id = $award_info_node->__toString();

        // todo: look in cache for award_info!
        $kuali_info = get_openskydora_kuali_award_info($award_id);
        if (isset($kuali_info['title']) && isset($kuali_info['sponsorName'])) {
            $award_markup = "<p>" . $kuali_info['sponsorName'].' #' . $award_id . ' - '.$kuali_info['title']. "</p>\n";
        } else {
            $award_markup = "<p>$award_id</p>\n";
        }

        $funding_info['#markup'] = $award_markup;
        $funding_info['sponsorName'] = $kuali_info['sponsorName'];
        $funding_info['title'] = $kuali_info['title'];
        $funding_info['award_id'] = $award_id;
        $funders[] = $funding_info;
    }
    return $funders;
}


/*
 * Islandora_scholar _pdf_upload_form_alter is used to upload PDF in the process
 * of creating a new object with forms. (see includes/pdf_upload.form.inc)

 * Here we simply remove the usage options from the form, and then replace the
 * scholar module's validation function
 * (islandora_scholar_pdf_upload_form_validate) with our own version that
 * doesn't require the usage options.
 */
function openskydora_form_islandora_scholar_pdf_upload_form_alter (&$form, &$form_state, $form_id) {
    //    remove $form['upload_document']['usage']
    unset ($form['upload_document']['usage']);

    // now change the validation function
    module_load_include ('inc', 'openskydora', 'includes/utilities');

    // insert openskydora validate before islandora_scholar_pdf_upload_form_validate
    //    (didn't work)
    //    array_splice ($form['hidden_next']['#validate'], 0, 0, 'openskydora_pdf_upload_form_validate');
  
    // replace islandora_scholar_pdf_upload_form_validate with openskydora version
    $form['hidden_next']['#validate'][0] = 'openskydora_pdf_upload_form_validate';
}

/*
 * This is a duplicate of islandora_scholar_pdf_upload_form_validate except we
 * don't validate the usage block, since we removed these fields from form (see
 * includes/pdf_upload.form.inc) 
 *
 * NOTE: I tried to do this in a more graceful way, but 
 */
function openskydora_pdf_upload_form_validate(&$form, &$form_state) {
    error_log ('islandora_scholar_pdf_upload_form_validate ...');
    if ($form_state['values']['upload_pdf_checkbox']) {
        if (empty($form_state['values']['file'])) {
            form_set_error('file', t('A file must be uploaded!'));
        }
        if (empty($form_state['values']['version'])) {
            form_set_error('version', t('A document version must be selected!'));
        }

        if (empty($form_state['values']['certifying']['certify'])) {
            form_set_error('certifying', t('You must certify that you have the right to upload this PDF!'));
        }
    } 
}

/**
 * Implement the ir_citationCModel_islandora_view_object hook so we
 * can display ir_citationCModel objects (rather than
 * islandora_scholar, which used strictly renderable arrays)
 */
function openskydora_ir_citationCModel_islandora_view_object($object, $page_number, $page_size) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    $output = theme('openskydora_citation', array('islandora_object' => $object));
    return array('' => $output);
}
// ---------- overriding scholar/includes/upload.tab.inc
/**
 * 
 * This form is accessed via the "Document" tab for a citation object (and others??). 
 *
The only thing we want to modify is:
$mods_versions = $mods_xpath->query('//m:mods/m:physicalDescription/m:note[@type="version"]');

The problem is that the alter function does not have immediate access to object and hence to 
the MODS Stream. We could get it via
*
*/
function openskydora_form_islandora_scholar_modify_form_alter (array &$form, array &$form_state) {

    // grab object from form_state
    $object = $form_state['build_info']['args'][0];
    $pid = $object->id;
    
    // get the version from the openskydora-specific location in the MODS stream
    $mods_str = trim($object['MODS']->content);

    $mods_doc = new DOMDocument();
    $mods_doc->loadXML($mods_str);
    $mods_xpath = new DOMXPath($mods_doc);
    $mods_xpath->registerNamespace('m', 'http://www.loc.gov/mods/v3');
    $mods_versions = $mods_xpath->query('//m:mods/m:physicalDescription/m:note[@type="version"]');
    
    if ($mods_versions->length > 0) {
        $version = $mods_versions->item(0)->textContent;
        $form['version']['#markup'] = $version;
    }
}

/**
 * Implements hook_islandora_scholar_upload_form_alter
 *
 * Just like in islandora_scholar_PDF_upload_form, we want to remove 
 * $form['upload_document']['usage'] from the form.
 * 
 * In this case, there is no validation function to modify, but we do replace
 * the submit function
*/
function openskydora_form_islandora_scholar_upload_form_alter (&$form, &$form_state, $form_id) {
    unset ($form['upload_document']['usage']);

    // we also want to change an aspect of this form's submit process (see
    // islandora_scholar_upload_form_submit).

    // We replace islandora_scholar_upload_submit because we need to change the way it 
    // modifies the MODS with values from the form.

    foreach ($form['#submit'] as $key=>$submit_fn) {
        if ($submit_fn == 'islandora_scholar_upload_form_submit') {
            $form['#submit'][$key] = 'openskydora_upload_form_submit';
        }}
}

/**

This version of the submit function modifies the MODS with
openskydora_add_usage_and_version_elements_to_mods rather than
islandora_scholar_add_usage_and_version_elements_to_mods
*/
function openskydora_upload_form_submit (array $form, array &$form_state) {
    extract($form_state['values']);

    $object = islandora_object_load($pid);

    try {
        $datastream = $object->constructDatastream($dsid, 'M');
        $datastream->label = 'PDF Datastream';
        $datastream->mimetype = 'application/pdf';
        $file = file_load($file);
        $path = drupal_realpath($file->uri);
        $datastream->setContentFromFile($path);

        $object->ingestDatastream($datastream);
        $usage = null;
        openskydora_add_usage_and_version_elements_to_mods($object, $usage, $version);
    }
    catch (Exception $e) {
        drupal_set_message(t('@message', array('@message' => check_plain($e->getMessage()))), 'error');
    }
}

/**
 * Replaces islandora_scholar_add_usage_and_version_elements_to_mods to
 * implement openskydora-specific ways of encoding usage and version values in MODS.
 */
function openskydora_add_usage_and_version_elements_to_mods (AbstractObject $object, $usage, $version) {

    $refuse_msg = t('Refusing to add usage and publication version statements alone');
    if (empty($object['MODS'])) {
        drupal_set_message(t('No MODS in object @pid. %refuse.', array('@pid' => $object->id, '%refuse' => $refuse_msg)), 'error');
        return;
    }
    $namespace = 'http://www.loc.gov/mods/v3';
    $doc = new DOMDocument();
    $doc->loadXML($object['MODS']->content);
    $xpath = new DOMXPath($doc);
    $xpath->registerNamespace('mods', $namespace);
    // Remove all instances of mods:accessCondition and mods:physicalDescription
    // from every mods:mods element instance.
    // OpenSky change
    $physical_description_query = '//mods:physicalDescription';
    $results = $xpath->query("$physical_description_query");
    foreach ($results as $result) {
        $result->parentNode->removeChild($result);
    }
    // Regardless of the number of mods:mods elements in the document we only
    // add the usage and version to the first one.
    $results = $xpath->query('//mods:mods[1]');
    if ($results->length == 0) {
        drupal_set_message(t('Could not find MODS element in object @pid is empty. %refuse.', array('@pid' => $object->id, '%refuse' => $refuse_msg)), 'error');
        return;
    }
    $mods_element = $results->item(0);

    #  // Add mods:physicalDescription to the first mods:mods element.
    $physical_description_element = $doc->createElementNS($namespace, 'physicalDescription');

    $physical_description_note_element = $doc->createElementNS($namespace, 'note');
    $physical_description_note_element->setAttribute('type', 'version');
    $physical_description_note_element->nodeValue = $version;
    $physical_description_element->appendChild($physical_description_note_element);
    $mods_element->appendChild($physical_description_element);

    // Commit changes to datastream.
    $object['MODS']->setContentFromString($doc->saveXML());
}

/* ----- islandora_scholar_embargo module alterations

   The two functions below encapsulate the openskydora modifications to the embargo
   module. 

   - openskydora_form_islandora_scholar_embargo_form_alter introduces some "Embargo
     Options" beyond those supplied, and it also replaces
     islandora_scholar_embargo_form_submit with openskydora_embargo_form_submit
     (which is a verbatim copy of the modified
     islandora_scholar_embargo_form_submit)

    - NOTE: the modified embargo did not work properly in its former location in
      islandora_scholar_embargo and it does not work properly here. Library
      personal report that PDF embargo might work, but other levels and
      durations of embargo are not used because they don't work properly. This
      is an area of functionality that will have to be fixed after OpenSky is
      hosted at Discovery Garden.
*/

/**
 * Recalculate the "Embargo Options" with openskydora-specific values. We allow:
 * - 'object'
 * - 'PDF'
 * - 'OBJ'
 * - 'PROXY_MP3'

these options are displayed only if a datastream with this ID is
present in current object.

 */
function openskydora_form_islandora_scholar_embargo_form_alter (&$form, &$form_state, $form_id) {
    $options = array(
        'object' => t('Object-level embargo'),
        'datastream' => t('Datastream'),
    );

    // add options for the object datastreams
    $pid = $form['pid']['#value'];
    $object = islandora_object_load($pid);
    foreach ($object as $datastream) {
        $options[$datastream->id] = $datastream->label;
    }

    // allowable options - these are the only "embargop options" allowed in opensky
    $allowed_options = array (
        'object' => '',
        'PDF' => '',    // OPenSky added PDF to original 
        'OBJ' => '',
        'PROXY_MP3' => '',
    );

    // reduce options to those allowable
    $options = array_intersect_key($options, $allowed_options);
    $form['dsid']['#options'] = $options;

    // replace submit function with openskydora version
    $form['#submit'] = array('openskydora_embargo_form_submit');
}

/**
 * We make enough openskydora-specific changes to
 * islandora_scholar_embargo_form_submit that we have to copy the islandora_scholar 
 * function here and modify it, thus replacing it with openskydora_embargo_form_submit
 */
function openskydora_embargo_form_submit (array $form, array &$form_state) {

    $embargoed = !empty($form_state['islandora_embargo']);
    $pid = $form_state['values']['pid'];
    $object = islandora_object_load($pid);
    // We are lifting an embargo from an object that already has one.
    if (isset($form_state['values']['update_options']) && $form_state['values']['update_options'] === 'lift_embargo') {
        $dsids = array();

        // This will have to be re-defined if and when we decide to allow
        // multiple separate embargoing on an object likely by using this
        // function: islandora_scholar_embargo_lift_associated_except(). For now
        // we can only have one embargo.

        foreach ($form_state['islandora_embargo'] as $embargo_vals) {
            $type = $embargo_vals['type'];
            if ($type !== 'Object') {
                $dsids[] = $type;
            }
            // OpenSky
            islandora_scholar_embargo_lift_embargo($pid, $dsids);
            // If an embargo on audio datastream, also remove the embargo on the
            // OBJ datastream
            if ($dsids == 'PROXY_MP3') {
                islandora_scholar_embargo_lift_embargo($pid, 'OBJ');
            }
            drupal_set_message(t('The embargo on @label (@pid) has been lifted!', array(
                '@label' => $object->label,
                '@pid' => $pid,
            )));
            // end OpenSky
        }
        // Batch down and down and down.
        if (isset($form_state['values']['recurse']) && $form_state['values']['recurse']) {
            $option = $form_state['values']['recurse_options'];
            $query_array = $form_state['query_choices'][$option];
            $batch = array(
                'title' => t('Lifting Embargoes'),
                'progress_message' => t('Please wait if many objects are being updated this could take a few minutes.'),
                'operations' => array(
                    array(
                        'islandora_scholar_embargo_lift_batch_function',
                        array($pid, $dsids, $query_array),
                    ),
                ),
                'finished' => 'islandora_scholar_embargo_lift_batch_finished',
                'file' => drupal_get_path('module', 'islandora_scholar_embargo') . '/includes/batch.inc',
            );
            batch_set($batch);
        }
        islandora_scholar_embargo_lift_embargo($pid, $dsids);
        drupal_set_message(t('The embargo on @label (@pid) has been lifted!', array(
            '@label' => $object->label,
            '@pid' => $pid,
        )));
    }
    elseif (isset($form_state['values']['update_date_options'])) {
        // Embargoed indefinitely.
        if ($form_state['values']['update_date_options'] === 'indefinite_embargo') {
            $end = 'indefinite';
        }
        else {
            $time = $form_state['values']['embargo_date'];
            $date_string = $time['year'] . '-' . $time['month'] . '-' . $time['day'];
            $end = gmdate("Y-m-d\TH:i:s\Z", strtotime($date_string));
        }

        // Object is currently embargoed and we are updating things.
        if ($embargoed) {
            $dsids = array();
            foreach ($form_state['islandora_embargo'] as $embargo_vals) {
                $type = $embargo_vals['type'];
                if ($embargo_vals['type'] == 'Object') {
                    $dsids = NULL;
                }
                else {
                    $dsids[] = $type;
                }
            }
        }
        // New embargo entry.
        else {
            if ($form_state['values']['dsid'] === 'object') {
                $dsids = NULL;
            }
            else {
                // OpenSky
                //        $dsid_options =
                //        array_filter($form_state['values']['ds_options']);
                //        $other_options = array();
                //        if ($form_state['values']['ds_options_other']) {
                //          $other_options =
                //        drupal_map_assoc(array_values(array_filter(preg_split('/[,|\s]/',
                //        trim($form_state['values']['ds_other'])))));
                //        }
                //        $dsids = array_unique(array_merge($dsid_options, $other_options));
                $dsids = (array)$form_state['values']['dsid'];
                // end OpenSky
            }
        }
        // Currently we just overwrite what's already in place. In the future, for
        // multiple embargoes on a single object, this will have to be adjusted.

        // If we're recursing do those objects in a batch.
        if (isset($form_state['values']['recurse']) && $form_state['values']['recurse'] && $form_state['values']['recurse_options'] != 'newchildren') {
            $option = $form_state['values']['recurse_options'];
            $query_array = $form_state['query_choices'][$option];
            $batch = array(
                'title' => t('Updating Embargoes'),
                'progress_message' => t('Please wait if many objects are being updated this could take a few minutes.'),
                'operations' => array(
                    array(
                        'islandora_scholar_embargo_apply_batch_function',
                        array($pid, $dsids, $end, $query_array),
                    ),
                ),
                'finished' => 'islandora_scholar_embargo_apply_batch_finished',
                'file' => drupal_get_path('module', 'islandora_scholar_embargo') . '/includes/batch.inc',
            );
            batch_set($batch);
        }
        islandora_scholar_embargo_set_embargo($pid, $dsids, $end);
        islandora_scholar_embargo_set_display_message($pid, $dsids, $end);
    }
}
// ------- openseadragon
function openskydora_template_process_islandora_openseadragon_viewer (array &$variables) {

    $library_path = libraries_get_path('openseadragon');
    $openseadragon_module_path = drupal_get_path('module', 'islandora_openseadragon');
    $openskydora_module_path = drupal_get_path('module', 'openskydora');

    drupal_add_js(array(
        'islandoraOpenSeadragon' => $variables['settings'],
    ), 'setting');

    drupal_add_js("$library_path/openseadragon.js", array('weight' => -4));
    // CHANGE JS FILE for OPENSKY!
    if (islandora_openseadragon_use_djatoka_server()) {
        //        drupal_add_js("$openseadragon_module_path/js/djtilesource.js", array('weight' => -3));
        drupal_add_js("$openskydora_module_path/js/openseadragon_djtilesource.js", array('weight' => -3));
    }
    drupal_add_js("$openseadragon_module_path/js/islandora_openseadragon.js", array('weight' => -2));
    drupal_add_css("$openseadragon_module_path/css/islandora_openseadragon.theme.css");
}

/* ------------- IP embargo
 * Implements hook_islandora_object_access().
we're trying to add the object's PID to the url so the embargo_message
can have just a little context
*/
function openskydora_islandora_object_access($op, $islandora_object, $user) {
  module_load_include('inc', 'islandora_ip_embargo', 'includes/utilities');
  // Handle an embargo.
  if (islandora_ip_embargo_restrict_access($islandora_object->id) && !user_access(ISLANDORA_IP_EMBARGO_MANAGE_EMBARGOES)) {
    // If there is a redirect configured then use it. If not let Drupal do it.
    $redirect_url = variable_get('islandora_ip_embargo_embargoed_redirect', '');
    if ($redirect_url) {
        if (strpos(current_path(), "islandora/object/{$islandora_object->id}") !== FALSE && strpos(current_path(), '/datastream/TN') === FALSE) {
            if (variable_get('islandora_ip_embargo_embargoed_redirect_append_url', 0)) {
                global $base_url;
                $redirect_target = $redirect_url . '/' . $islandora_object->id;
                drupal_goto($redirect_target);
            }
            else {
                drupal_goto($redirect_url);
            }
        }
        else {
            return FALSE;
        }
    }
    else {
        return FALSE;
    }
  }
}

/**
Alter the prompts on the IP embargo config. we are appending the
objects PID to the url, NOT the object's whole URL
*/
function openskydora_form_islandora_ip_embargo_settings_form_alter(array &$form, array &$form_state) {
    $form['islandora_ip_embargo_embargoed_redirect_append_url']['#title'] = 'Append embargoed object\'s PID';
    $form['islandora_ip_embargo_embargoed_redirect_append_url']['#description'] = 'Check this option if you want to append the embargoed object\'s URL';
}
