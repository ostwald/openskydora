<?php

function template_preprocess_jlo_home_mockup(&$variables) {
    drupal_add_js(drupal_get_path('module', 'jlo') . '/js/base-script.js');    
    drupal_add_js(drupal_get_path('module', 'jlo') . '/js/home_page_mockup.js');
    drupal_add_css(drupal_get_path('module', 'jlo') . '/css/home-mockup.css');
    
    $variables['img_dir'] = drupal_get_path('module', 'jlo') . '/images';
    $variables['show_counts'] = 1;

    module_load_include('inc', 'islandora_usage_stats', 'includes/db');
    $top_searched = islandora_usage_stats_get_top_searched(10);
    $variables['top_searched'] = $top_searched;
    //    dsm($top_searched);

    $variables['image_carousel_pids'] = array_map (function ($a) {
        return trim($a);
    }, explode("\n", variable_get('image_carousel_pids', '')));
    
}

function template_preprocess_jlo_wrapper (&$variables) {
    module_load_include('inc', 'islandora_usage_stats', 'includes/db');
    $variables['top_searches'] = islandora_usage_stats_get_top_searched (25);
    
    module_load_include('inc', 'jlo', 'includes/utilities');
    module_load_include('inc', 'jlo', 'includes/usage_stats');

    //    jlo_get_most_viewed_authors();
    
    // NEW CONTENT MODEL ITEMS - using general jlo_get_new_items
    $args = array ('RELS_EXT_hasModel_uri_s'=>'info:fedora/ir:citationCModel');
    //    $args = array ('RELS_EXT_hasModel_uri_t'=>'citation');
    $new_citation_items = jlo_get_new_items(5, 50, $args);
    // dsm($new_citation_items);

    // NEW COLLECTION ITEMS
    $args = array ('RELS_EXT_isMemberOfCollection_uri_s'=>'info:fedora/islandora:sr');
    $new_collection_items = jlo_get_new_items(5, 50, $args);
    //     dsm($new_collection_items);
    
    // dsm (jlo_get_new_images(10, 50));
    
    // MOST VIEWED COLLECTIONS

    $most_viewed_collections_weeks_ago = 5;
    $most_viewed_collections_count = 3;
    $variables['most_viewed_collections_weeks_ago'] = $most_viewed_collections_weeks_ago;
    $variables['most_viewed_collections'] = jlo_get_most_viewed_collections(
        $most_viewed_collections_count,
        $most_viewed_collections_weeks_ago);
    
    if (1) {  // SHOW DATABASE FIELDS
        $db_names = array (
            //'islandora_usage_stats_object_ds_access_log',
            //            'islandora_usage_stats_object_access_log',
            //'islandora_usage_stats_objects',
            //'islandora_usage_stats_datastreams',
            'islandora_usage_stats_top_searches',
        );

        foreach ($db_names as $db_name) {
            $objects = show_db_fields($db_name);
            dsm("$db_name fields");
            dsm($objects);
        }
    }

    if (1) {  // MOST VIEWED

        $most_viewed_items_weeks_ago = 25;
        $most_viewed_items_count = 5;
        $exclude = array('opensky:root');
        $most_viewed_items = jlo_get_most_viewed_items(
            $most_viewed_items_count,
            $most_viewed_items_weeks_ago,
            $exclude);
        $variables['most_viewed_items'] = $most_viewed_items;
        $variables['most_viewed_items_weeks_ago'] = $most_viewed_items_weeks_ago;
    }

    if (1) { // MOST DOWNLOADED
        $most_downloaded_weeks_ago = 25;
        $most_downloaded_count = 5;
        $variables['most_downloaded_weeks_ago'] = $most_downloaded_weeks_ago;
        $variables['most_downloaded'] = jlo_get_most_downloaded (
            $most_downloaded_count,
            $most_downloaded_weeks_ago);
    }
}

function template_preprocess_jlo_checksum_results (&$variables) {
    module_load_include('inc', 'jlo', 'includes/utilities');
    module_load_include('inc', 'islandora_checksum', 'includes/checksum');

    $pid = @$_GET['pid'];
    $scope = $_GET['scope'];
    $resp = null;
    if ($pid && $scope='object') {
        try {
            islandora_checksum_set_checksums($pid);
        } catch (Exception $e) {
            $msg = 'Caught exception: ' .  $e->getMessage();
            // dsm($msg);
            // echo ($msg);
            $variables['error'] = $msg;
        }
        /*
          here's where we load up $variables
        */
        $variables['pid'] = $pid;
        $variables['scope'] = $scope;
    }
}

// COLLECTIONS -----------------

function template_preprocess_jlo_collection(&$variables) {

    module_load_include('inc', 'islandora', 'includes/utilities');
    module_load_include('inc', 'openskydora', 'includes/utilities');
    module_load_include('inc', 'jlo', 'includes/collections');

    $pid = $variables['pid'];
    //    watchdog ('jlo', 'template_preprocess_jlo_collection -- ' . $pid);
    $obj = openskydora_get_fedora_object($pid);
    $parent_collections = islandora_get_parents_from_rels_ext ($obj);
    $parent = empty ($parent_collections) ? null : $parent_collections[0];
    
    $tree = openskydora_get_collection_tree();
    $infos = openskydora_get_pid_subcollection_infos($parent);

    foreach ($infos as $info) {

        if ($info['pid'] == $pid) {
            $jlo_collection = jlo_collections_get_collection($pid);
            $jlo_collection['children'] = $tree[$pid];
            $jlo_collection['landing_page'] = '/islandora/object/' . $pid;
            $jlo_collection['parent'] = $parent;
            $variables['jlo_collection'] = $jlo_collection;
        }
        if (isset ($variables['jlo_collection'])) {
            break;
        }
    }
    $base_path = drupal_get_path('module', 'jlo') . '/images';
    $image_vars = array (
        'path'=>$base_path . '/open.png',
        'width'=>'30px',
    );
    $variables['open_icon'] = theme('image', $image_vars);
    $image_vars['path'] = "$base_path/closed.png";
    $variables['closed_icon'] = theme('image', $image_vars) ;

}

function template_preprocess_jlo_collections(&$variables) {
    $jlo_module_path = drupal_get_path('module', 'jlo');
    drupal_add_js ("$jlo_module_path/js/base-script.js");
    // drupal_add_js ("$jlo_module_path/js/collections.js");
    drupal_add_js ("$jlo_module_path/js/collections-decorator.js");
    drupal_add_css ("$jlo_module_path/css/collections.css");

    $jlo_collections = jlo_collections_get_collections();
    $variables['jlo_collections'] = $jlo_collections;

    $base_path = drupal_get_path('module', 'jlo') . '/images';
    $image_vars = array (
        'path'=>$base_path . '/open.png',
        'width'=>'30px',
    );
    $variables['open_icon'] = theme('image', $image_vars);
    $image_vars['path'] = "$base_path/closed.png";
    $variables['closed_icon'] = theme('image', $image_vars) ;
    $variables['root_pid'] = ROOT_PID;
    //    $variables['test_tree'] = theme ('jlo_collection', array ('pid'=>'islandora:jonscollection'));
}                


// new stuff follows ---------------------------

function process_pid_as_collection (&$variables) {

    module_load_include('module', 'islandora_checksum', 'islandora_checksum');
    
    $collection = $_GET['pid'];
    
    if (isset($collection)) {
        
        $context = array();
        $limit = 10;
        $times = 0;
        while ($times < 500) {
            
            jlo_batch_operation_callback($collection, $limit, $context);
            if (isset($context['message'])) {
                // dsm ($context['message'])
                watchdog ('jlo', $context['message']);
                // dsm ($context['message']);
            }
            
            if (@$context['finished'] == 1) {
                break;
            }
            
            $times += 1;
            // watchdog ('jlo', "times: $times");
            if ($times > 10000) {
                watchdog('jlo', "broke after 10");
                break;
            }
        }
        
        $query_results = islandora_checksum_get_collection_member_details($collection, FALSE, 1000);
        // dsm ($query_results);
        
        $variables['collection'] = $collection;
        $variables['batch_results'] = $context['results'];
        $variables['total_count'] = $context['sandbox']['total'];
        $variables['query_results'] = $query_results;
    } else {
        $variables['collection'] = '';
        $variables['batch_results'] = array();
        $variables['total_count'] = 0;
        $variables['query_results'] = array();
    }
}

function template_preprocess_jlo_checksums_wrapper (&$variables) {

    module_load_include('module', 'islandora_checksum', 'islandora_checksum');
    // drupal_set_message ('template_preprocess_jlo_wrapper');

    // batch processor
    // $collection = 'archives:asr';
    // $collection = 'archives:unoh';

    // drupal_set_message('pid: ' . $_GET['pid']);

    if (false) {   // Process pid as a collection
        process_pid_as_collection ($variables);

    }
}
    
    
/**
 * Batch process 
 *
 * @param string $collection
 *   The PID of the collection.
 * @param int $limit
 *   The number of items to do per-iteration.
 * @param array $dsids
 *   The datastream DSIDs to target.
 * @param array $context
 *   The batch context.
 */
function jlo_batch_operation_callback($collection, $limit, &$context) {
    // watchdog ('jlo', 'Entering callback');
    $sandbox = &$context['sandbox'];
    // module_load_include('inc', 'islandora_checksum', 'includes/checksum');
    if (empty($sandbox)) {
        $sandbox['total'] = islandora_checksum_get_collection_member_details($collection, TRUE);

        if ($sandbox['total'] == 0) {
            $context['message'] = t('No objects to batch process.');
            $context['finished'] = 1;
            return;
        }
        $sandbox['current'] = 0;
        $sandbox['offset_params'] = NULL;

    }
    
    $slice = islandora_checksum_get_collection_member_details($collection, FALSE, $limit, $sandbox['offset_params']);
    watchdog('jlo', 'slice has ' . count($slice));
    
    foreach ($slice as $member) {
        // do the work on this item

        
        $sandbox['current']++;
        $sandbox['offset_params'] = array(
            'offset_pid' => $member['object']['value'],
            'offset_date' => $member['date']['value'],
        );
        $context['results'][] = $member['object']['value'];
        watchdog ('jlo', $member['object']['value']);
    }
    
    $context['message'] = t('Batch Processed for @current/@total objects.', array(
        '@current' => $sandbox['current'],
        '@total' => $sandbox['total'],
    ));
    
    // jlo - added 3/30 - emergency stop for runaway batch
    if (count($slice) == 0) {
        //  watchdog ('jlo', 'ATTEMPTING TO STOP BATCH');
        $context['finished'] = 1;
    }
    else {
        $context['finished'] = $sandbox['current'] / $sandbox['total'];
    }
    // watchdog ('jlo', 'exiting from callback_batch_operation: ' . $sandbox['current'] . '/' . $sandbox['total']);
    //watchdog ('jlo', 'exiting from islandora_checksum_enable_object_checksums for ' . $collection);
}
    
// -------------------

/**
 * Implements template_preprocess_HOOK().
 *
 */
function template_preprocess_jlo_home_page(&$variables) {
    module_load_include('inc', 'openskydora', 'includes/utilities');
    $variables['tree'] = openskydora_get_collection_tree();
    $variables['img_dir'] = drupal_get_path('module', 'jlo') . '/images';

    // ISLANDORA_USAGE_STATS
    /*
    module_load_include('inc', 'islandora_usage_stats', 'includes/db');

    $variables['top_downloads'] = islandora_usage_stats_get_top_downloads(5);
    $variables['most_viewed'] = islandora_usage_stats_get_most_viewed(5);
    $variables['recently_viewed'] = islandora_usage_stats_get_recently_viewed(5);
    $variables['top_searched'] = islandora_usage_stats_get_top_searched(5);
    */

}
