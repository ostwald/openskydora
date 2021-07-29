<?php
/* 
   islandora_usage_stats_objects has fields: id, label, pid

*/

module_load_include('inc', 'islandora_usage_stats', 'includes/db');

// ------ how-tos ------------
// get pid_id from pid
//    $pid_id = islandora_usage_stats_get_pid_id($pid);
//    dsm('pid_id:' . $pid_id);

// get timestamp in the past
//    dsm('time last week:' . strtotime("-1 week"));
// -------------------

function show_db_fields($db_name) {
    $query = db_select($db_name, 'db')
           ->fields('db')
           ->range(0,1);
    $results = $query->execute()->fetchAll();
    return $results;
}

function pid_query_clause ($pid) {
    return "PID:\"$pid\"";
}
/*
  usage:
    $timestamp = strtotime("-$weeks_ago week");
    $query = 'keyDate:[' . get_ISO_date_string($timestamp)
    . ' TO *]';
*/
function get_ISO_date_string ($timestamp) {
    return date("Y-m-d\TH:i:s", $timestamp) .'.000Z';
}

function ISO_to_human_readable($iso_date_string) {
    return date('Y-m-d', strtotime($iso_date_string));
}

function openskydora_get_new_images($count=5, $weeks_ago=5) {
    $args = array ('RELS_EXT_hasModel_uri_t'=>'image');
    return openskydora_get_new_items($count, $weeks_ago, $args);
}

function openskydora_get_most_viewed_authors($count=5, $weeks_ago=5) {
    // this stat is based on views, so get the recently viewed objects
    // (could we narrow to articles?)
     $views = openskydora_get_most_viewed (10000, $weeks_ago);
     dsm ('processing '. count($views) . ' hits');
     dsm($views);
     $query = implode (array_map ('pid_query_clause', array_keys($views)), ' OR ');
     $solr_fields = array(
         'PID',
         'fgs_createdDate_dt',
         'fgs_label_s',
         'mods_extension_collectionKey_ms',
         'RELS_EXT_isMemberOfCollection_uri_s',
         'RELS_EXT_hasModel_uri_s',
         'mods_name_personal_author_nameIdentifier_UPID_ms'
     );
    $params = array(
        'fl'=>implode($solr_fields, ','),
        'sort'=>'fgs_createdDate_dt desc',
        'solrLimit'=>$count,
    );
  
    $query_results = openskydora_solr_search($query, $params);
    dsm("results");
    dsm ($query_results);
}
     
function openskydora_get_new_items($count=5, $weeks_ago=5, $args=NULL) {
    $timestamp = strtotime("-$weeks_ago week");
    $query = 'fgs_createdDate_dt:[' . get_ISO_date_string($timestamp) . ' TO *]';

    if ($args) {
        foreach($args as $key=>$value) {
            $query .= " AND ($key:\"$value\")";
        }
    }
    $solr_fields = array(
        'PID',
        'fgs_createdDate_dt',
        'fgs_label_s',
        'RELS_EXT_isMemberOfCollection_uri_s',
        'RELS_EXT_hasModel_uri_s'
    );
    $params = array(
        'fl'=>implode($solr_fields, ','),
        'sort'=>'fgs_createdDate_dt desc',
        'solrLimit'=>$count,
    );
    
    $query_results = openskydora_solr_search($query, $params);

    $ret = array();
    foreach($query_results as $item) {
        $ret[] = array(
            'pid' => $item['PID'],
            'collection' => $item['RELS_EXT_isMemberOfCollection_uri_s'],
            'label' => $item['fgs_label_s'],
            'created' => ISO_to_human_readable($item['fgs_createdDate_dt']),
            'model' => $item['RELS_EXT_hasModel_uri_s'],
        );
    }
    return $ret;
}

/*
  get list of all views in past $weeks_ago and then
  use solr search to filter by collection
*/
function openskydora_get_most_viewed_collections($count=5, $weeks_ago=5) {
    // get all the views in the window
    $views = openskydora_get_most_viewed (10000, $weeks_ago);

    // dsm($views);

    // search for all the unique PIDs that aren't collections
    $query = implode (array_map ('pid_query_clause', array_keys($views)), ' OR ');
    $query = '('.$query.') NOT RELS_EXT_hasModel_uri_t:"*collectionCModel"';
    $solr_fields = array(
        'PID',
        'fgs_createdDate_dt',
        'fgs_label_s',
        'mods_extension_collectionKey_ms',
        'RELS_EXT_isMemberOfCollection_uri_s',
        'RELS_EXT_hasModel_uri_s'
    );
    $params = array(
        'fl'=>implode($solr_fields, ','),
        'sort'=>'fgs_createdDate_dt desc',
        'solrLimit'=>count($views),
    );
    $query_results = openskydora_solr_search($query, $params);
    // dsm("results");
    // dsm ($query_results);

    // construct most_viewed array to return
    $most_viewed = array();
    foreach($query_results as $item) {
        $pid = $item['PID'];
        //dsm($pid . ' views: ' . $views[$pid]->views);
        $collection = $item['RELS_EXT_isMemberOfCollection_uri_s'];
        $label = $item['fgs_label_s'];
        $coll_views = isset($most_viewed[$collection]) ? $most_viewed[$collection] : 0;
        $coll_views += intval($views[$pid]->views);
        // dsm ("- $collection coll_views: $coll_views");
        /*        $most_viewed[$collection] = array (
            'views' =>$coll_views,
            'label' => $label
        );
        */
        $most_viewed[$collection] = $coll_views;
    }

    arsort($most_viewed);
    /*
    usort($most_viewed, function ($a, $b) {
        return $a['views'] <=> $b['views'];
    });
    */
    return array_slice($most_viewed, 0, $count, TRUE);
}


// -------------- VIEWS --------------
/**
 * Get a number of the most recently viewed objects in a specified
 * time window.
 *
 * @param int $count
 *   The number of objects to get.
 * @param array $exclude
 *   A list of PIDs to exclude.
 * @param int $weeks_ago
 *   limit results to the last $weeks_ago
 *
 * @return array
 *   A list of results containing an object with attributes of pid and
 *   views.
 */
function openskydora_get_most_viewed_items($count, $weeks_ago=7, $exclude=NULL) {

    $records = openskydora_get_most_viewed($count, $weeks_ago, $exclude);
    $item_records = array();  // No collections
    
    // filter out collection citation model
    foreach ($records as $pid=>$data) {
        $data->obj = islandora_object_load($pid);
        if (in_array('islandora:collectionCModel',$data->obj->models)) {
            // dsm ('collection! '. $pid);
        } else {
            $item_records[$pid] = $data;
        }
    }
    return array_slice($item_records, 0, $count, TRUE);
}

/**
 * Get a number of the most recently viewed objects in a specified
 * time window.
 *
 * @param int $count
 *   The number of objects to get.
 * @param array $exclude
 *   A list of PIDs to exclude.
 * @param int $weeks_ago
 *   limit results to the last $weeks_ago
 *
 * @return array
 *   A list of results containing an object with attributes of pid and
 *   views.
 */
function openskydora_get_most_viewed($count, $weeks_ago=7, $exclude=NULL) {
    $query = db_select('islandora_usage_stats_object_access_log', 'log');
    $query->join('islandora_usage_stats_objects', 'objects', 'log.pid_id = objects.id');
    $query->addExpression('COUNT(log.pid_id)', 'views');
    $query->fields('objects', array('pid'))
        ->groupBy('log.pid_id')
        ->orderBy('views', 'DESC')
        ->range(0, $count);
    // XXX: This could potentially cause slow down on large data sets.
    if ($exclude) {
        $query->condition('pid', $exclude, 'NOT IN');
    }
    if ($weeks_ago) {
        $query->condition('log.time', strtotime("-$weeks_ago week"), '>');
    }
    $results = $query->execute();
    $records = $results->fetchAllAssoc('pid');
    return $records;
}

// --------- DOWNLOADS --------------

function openskydora_get_most_downloaded ($count=5, $weeks_ago=25) {
    //dsm ("openskydora_get_most_downloaded() - weeks ago: $weeks_ago\n");
    $query = db_select('islandora_usage_stats_object_ds_access_log', 'log');
    $query->join('islandora_usage_stats_datastreams', 'datastreams', 'log.ds_id = datastreams.id');
    $query->join('islandora_usage_stats_objects', 'objects', 'datastreams.pid_id = objects.id');

    $query->addExpression('COUNT(log.ds_id)', 'downloads');
    
    if ($weeks_ago) {
        $query->condition('log.time', strtotime("-$weeks_ago week"), '>');
    }

    $results = $query->fields('objects', array('pid', 'label'))
             ->groupBy('log.ds_id')
             ->orderBy('downloads', 'DESC')
             ->range(0, $count)
             ->execute();

    return $results->fetchAll();

}
