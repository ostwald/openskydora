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
    //dsm($variables);
    // $pid = openskydora_get_pid_from_request();

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

    drupal_set_message ('parent: ' . $variables['parent_collections'][0]);

    $embargo_msgs = array(
        //        'islandora:jonscollection' => 'default',
        'islandora:video_collection' => 'video'
    );
        
    foreach ($embargo_msgs as $pid => $name) {
        if (in_array('islandora:video_collection', $variables['parent_collections'])) {
            $options = array('query' => array ('pid' => $pid,));
            $goto = "embargo/$name";
            drupal_goto ($goto, $options);
            return;
        }
    }
    
    if (!in_array('islandora:jonscollection', $variables['parent_collections'])) {
        $options = array('query' => array ('pid' => $pid,1));
        $goto = "embargo/default";
        drupal_set_message ("GOTO $goto");
        drupal_goto ($goto, $options);
    }
    
}
