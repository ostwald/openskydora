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

    module_load_include('inc', 'openskydora', 'includes/utilities');
    if (!openskydora_has_subcollections($islandora_object->id)) {
        openskydora_redirect_to_search_view($islandora_object->id);
        return;
    }
}
