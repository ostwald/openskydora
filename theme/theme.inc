<?php

/**
 * @file
 * This file contains all theme and preprocess functions
 */

/**
 * Implements template_preprocess_HOOK().
 *
 */
function template_preprocess_openskydora_scratch(&$variables) {
    $pid = $variables['pid'];
    $dc_array = openskydora_get_dc_array($pid);
    $variables['title'] = $dc_array['dc:title']['value'];
    $variables['children'] = openskydora_get_pid_subcollections_pids($pid);
    $variables['descendants'] = openskydora_get_subcollections_recursive($pid);
    $variables['searchables'] = openskydora_get_searchable_subcollections($pid);
    $obj = openskydora_get_fedora_object($pid);
    $variables['tree'] = openskydora_get_collection_tree();
}

/**
 * Implements template_preprocess_HOOK().
 * If this collection does not contain subcollections, then display it as
 * search results.
 */
function template_preprocess_openskydora_basic_collection_wrapper(&$variables) {
    $islandora_object = $variables['islandora_object'];

    /* 
    // figure out why the following is here before enabling it ...
    if (openskydora_islandora_object_access ("", $islandora_object, $variables['user']) === FALSE) {
        return false;
    }
    */

    module_load_include('inc', 'openskydora', 'includes/utilities');
    if (!openskydora_has_subcollections($islandora_object->id)) {
        // we don't want to redirect if there is more to the url
        // e.g., /islandora/objects/islandora%3Ajonscollection/datastream/TN/view

        //dsm ("redirecting to search view");
        //dsm( current_path());
        openskydora_redirect_to_search_view($islandora_object->id);
        return;
    }
}

/**
 * Implements template_preprocess_HOOK().

 * The template will need info necessary to choose a section of html:
 - object
 - parent_collections
 */
function template_preprocess_openskydora_embargo_messenger(&$variables) {

    module_load_include('inc', 'islandora', 'includes/utilities');

    $pid = $variables['pid'];
    $object = openskydora_get_fedora_object($pid);
    $variables['object'] = $object;
    if ($object) {
        $parent_collections = islandora_get_parents_from_rels_ext($object);
        $variables['parent_collections'] = $parent_collections;
    }
    else {
        $variables['parent_collections'] = array ();
    }

    // associate a collection pid with the name of the embargo message page
    $embargo_msgs = array(
        'archives:ucarawards' => 'ucarawards',
    );
        
    foreach ($embargo_msgs as $pid => $name) {
        if (in_array($pid, $variables['parent_collections'])) {
            $options = array('query' => array ('pid' => $pid,));
            $goto = "embargo/$name";
            drupal_goto ($goto, $options);
            return;
        }
    }

    // if we're still here, that means we haven't found a
    // collection-specific embarbo page. recirect to the default
    // embargo message.
   
    $options = array('query' => array ('pid' => $pid,1));
    $goto = "embargo/default";
    // drupal_set_message ("GOTO $goto");
    drupal_goto ($goto, $options);
   
}

/**
 * display funding information based in award id, and related dataset
 * or software items
 */
function template_process_openskydora_metadata_display(array &$variables) {

    if (isset($variables['solr_fields']['mods_note_funding_s'])) {
        $award_markup = array();
        $award_ids = $variables['solr_fields']['mods_note_funding_s']['value'];
        foreach($award_ids as $award_id) {
            // todo: look in cache for award_info!
            $kuali_info = get_kuali_award_info($award_id);
            $award_markup[] = "<div>" . $kuali_info['title'].'  (' . $kuali_info['sponsorName'] . " #$award_id)</div>\n";
            $variables['solr_fields']['mods_note_funding_s']['award_markup'] = implode($award_markup);
        }
    }

    // place holder for any manipulation we want to do for related
    // DataSet or software ...
    if (isset($variables['solr_fields']['mods_relatedItem_genre_authorityURI_ms'])) {
        //dsm($variables['solr_fields']['mods_relatedItem_genre_authorityURI_ms']);
        $dataset_markup = array();
    }
    
}
