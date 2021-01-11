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
 * Implements template_preprocess_HOOK().
 *
 * Support display the Details (configured solr_fields). Provide field-specific
 * views for funding_info, authors, contributore
 */
function template_process_openskydora_metadata_display(array &$variables) {
    $islandora_object = $variables['islandora_object'];
    $funders = openskydora_get_funding_info($islandora_object);

    $funding_info_field_name = 'mods_note_funding_ms';
    $variables['funding_info_field_name'] = $funding_info_field_name;

    if (isset($variables['solr_fields'][$funding_info_field_name])) {
        $award_markup = array();
        $award_ids = $variables['solr_fields'][$funding_info_field_name]['value'];

        foreach($award_ids as $award_id) {
            $kuali_info = get_openskydora_kuali_award_info($award_id);
            
            if (isset($kuali_info['title']) && isset($kuali_info['sponsorName'])) {
                $award_markup[] = "<p>" . $kuali_info['sponsorName'] . ' #'.$award_id.' - '.$kuali_info['title']."</p>\n";
            } else {
                $award_markup[] = "<p>$award_id</p>\n";
            }
	    
            $variables['solr_fields'][$funding_info_field_name]['award_markup'] = implode($award_markup);
        }
    }

    /* authors */
    $authors_field_name = 'author_Display_with_Affiliation';
    if (isset($variables['solr_fields'][$authors_field_name])) {
        $authors_markup = '';
	$authors = $variables['solr_fields'][$authors_field_name]['value'];

	$authors_count = 0;
	$total = count($authors);

	foreach ($authors as $author) {
          if ($authors_count == 5) {
            $authors_markup .= '<span id="authors_more" class="hiddenContent">';
	  }
	  $authors_markup .= '<p>'.$author.'</p>';

          if ($authors_count == $total-1 && $authors_count >= 5) {
            $authors_markup .= '</span><button id="authors_more_btn" class="closed" aria-label="Show more or less authors toggle" onclick="toggleContent(\'authors_more\')">'.t('Show more').'</button>';
	  }
	  $authors_count++;
	}
	$variables['solr_fields'][$authors_field_name]['authors_markup'] = $authors_markup;

    }

    /* contributors */
    $contrib_field_name = 'contributor_Display_with_Affiliation';
    if (isset($variables['solr_fields'][$contrib_field_name])){
	    $people_markup = "";
	    $people = $variables['solr_fields'][$contrib_field_name]['value'];
	    	    
	    $people_count = 0;
	    $total = count($people);
	 
	    foreach($people as $person){
	       if($people_count == 5){
	         $people_markup .= '<span id="contrib_more" class="hiddenContent">';
	       }
	       $people_markup .= '<p>'.$person.'</p>';

	       if($people_count == $total-1 && $people_count >= 5){
                 $people_markup .= '</span><button id="contrib_more_btn" class="closed" aria-label="show more or less contributors toggle" onclick="toggleContent(\'contrib_more\')">'.t('Show more').'</button>';
	       }
	       $people_count++;
	    }

	    $variables['solr_fields'][$contrib_field_name]['people_markup'] = $people_markup;  
    }

    // Remove the Display_Name if we are not a pdf content_model)
    if (!in_array('islandora:sp_pdf', $islandora_object->models) && isset ($variables['solr_fields']['Display_Name'])) {
        unset ($variables['solr_fields']['Display_Name']);
    }
}

/**
 * Implements template_preprocess_HOOK()
 *
 */
function template_preprocess_openskydora_citation (array &$variables) {

    drupal_add_js('misc/form.js');
    drupal_add_js('misc/collapse.js');
    $islandora_object = $variables['islandora_object'];
    $repository = $islandora_object->repository;
    module_load_include('inc', 'islandora', 'includes/datastream');
    module_load_include('inc', 'islandora', 'includes/utilities');
    module_load_include('inc', 'islandora', 'includes/metadata');

    // We should eventually remove the DC object and dc_array code as
    // it only exists to not break legacy implementations.
    try {
        if (isset($islandora_object['DC']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DC'])) {
            $dc = $islandora_object['DC']->content;
            $dc_object = DublinCore::importFromXMLString($dc);
        }
    }
    catch (Exception $e) {
        drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
    }
    $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;
    $variables['dc_array'] = isset($dc_object) ? $dc_object->asArray() : array();
    $variables['islandora_object_label'] = $islandora_object->label;
    $variables['theme_hook_suggestions'][] = 'islandora_pdf__' . str_replace(':', '_', $islandora_object->id);
    $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);
    $variables['metadata'] = islandora_retrieve_metadata_markup($islandora_object);
    $variables['description'] = islandora_retrieve_description_markup($islandora_object);
    
    global $base_url;

    // Download swiped from _alter
    if (isset($islandora_object['PDF']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['PDF'])) {
        $filename = str_replace(":", "_", $islandora_object->id);
        $path = "islandora/object/$islandora_object->id/datastream/PDF/download/citation.pdf";
        $options['attributes']['class'] = array('islandora-pdf-link');
        $link = l('PDF', $path, $options);
        $variables['islandora_download_link'] = '<div class="openskydora-info">' . t('Download ') . $link . '</div>';
    }

    
    // Full size url.
    if (isset($islandora_object['PDF']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['PDF'])) {
        $full_size_url = url("islandora/object/{$islandora_object->id}/datastream/PDF/view", array('absolute' => TRUE));
        $variables['islandora_full_url'] = $full_size_url;

        // Sanitize this object name a bit and provide download link.
        $sanitized_label = preg_replace('/[^A-Za-z0-9_\-]|\.pdf$/', '_', $islandora_object->label);
        $download_url = 'islandora/object/' . $islandora_object->id . '/datastream/OBJ/download/' . $sanitized_label . '.pdf';
        $download_text = t("Download pdf");
        $variables['islandora_download_link'] = l($download_text, $download_url, array('attributes' => array('class' => array('islandora-pdf-link'))));
    }

    // Thumbnail.
    if (isset($islandora_object['TN'])) {
        $thumbnail_size_url = url("islandora/object/{$islandora_object->id}/datastream/TN/view");
        $params = array(
            'title' => $islandora_object->label,
            'path' => $thumbnail_size_url,
        );
        $variables['islandora_thumbnail_img'] = theme('image', $params);
    }

    // Preview image + link.
    if (isset($islandora_object['PREVIEW']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['PREVIEW'])) {
        $preview_url = url("islandora/object/{$islandora_object->id}/datastream/PREVIEW/view");
        $params = array(
            // 'title' => $islandora_object->label,
            'title' => 'Click to view',
            'path' => $preview_url,
        );

        $variables['islandora_preview_img'] = theme('image', $params);
        $variables['islandora_content'] = isset($variables['islandora_full_url']) ?
                                        l($variables['islandora_preview_img'], $variables['islandora_full_url'], array('html' => TRUE)) :
                                        $variables['islandora_preview_img'];
    }

    // CITATION 
    if (isset($islandora_object['MODS']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['MODS'])) {
        module_load_include('inc', 'islandora_scholar', 'includes/csl_select.form');
        $display['citation_select'] = drupal_get_form('islandora_scholar_citation_select_form', $islandora_object->id);
        $variables['citation'] = citeproc_bibliography_from_mods(
            citeproc_default_style(), $islandora_object['MODS']->content
        );
    }

    // DOI LINK
    $variables['doi_link'] = openskydora_retrieve_doi_markup($islandora_object);
    
    //    dsm ($variables);
    
}


