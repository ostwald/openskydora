<?php

// Utilities go here
function jlo_get_invokers($hook) {
    $return = array();
    foreach (module_implements($hook) as $module) {
        $return[] =  $module . '_' . $hook;
    }
    return $return;
}

/**
 * Implements hook_islandora_solr_query
 *
 * This is necessary to make our solrParams stick - otherwise they
 * will be overwritten by another islandora_solr_query hook.
 *
 * NOTE: this hook only acts when internalSolrParams['type'] is
 * recognized (e.g., when it is jlo).
*/
function jlo_islandora_solr_query($isqp) {
    // isqp is IslandoraSolrQqueryProcessor instance
    if (@$isqp->internalSolrParams['type'] == 'jlo') {
        $isqp->solrParams['fl'] = $isqp->internalSolrParams['fl'];
        if (isset($isqp->internalSolrParams['solrLimit'])) {
            $isqp->solrLimit = $isqp->internalSolrParams['solrLimit'];
        }
        // dsm($isqp);
    }
}

/*
  example params:
    $query = 'mods_extension_collectionKey_ms:"demo"';
    $params = array(
        'fl'=>'PID,fgs_createdDate_dt,fgs_label_s',
        'type'=>'jlo',
        'sort'=>'fgs_createdDate_dt desc',
    );

  NOTE: 'type' param is necessary (see jlo_islandora_solr_query), but
  it set explicitly so it need not be passed.
*/
function jlo_solr_search($query, $params) {
    module_load_include('inc', 'islandora_solr', 'includes/query_processor');
    //    dsm (jlo_get_invokers('islandora_solr_query'));
    
    if (!isset($params['type'])) {
        $params['type'] = 'jlo';
    }
        
    $qp = new IslandoraSolrQueryProcessor();
    $qp->buildQuery($query, $params);
    //dsm($qp);
        
    // THIS is straight outta executeQuery()
    $path_parts = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
    $solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path'] . '/');
    $solr->setCreateDocuments(0);
    
    // Query is executed.
    try {
      $solr_query = ($qp->internalSolrQuery) ? $qp->internalSolrQuery : $qp->solrQuery;
      $method = 'GET';
      $results = $solr->search($solr_query, $qp->solrStart, $qp->solrLimit, $qp->solrParams, $method);
    }
    catch (Exception $e) {
      drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $e->getMessage(), 'error');
    }

    $object_results = array();
    if (isset($results)) {
      $solr_results = json_decode($results->getRawResponse(), TRUE);
      //dsm($solr_results);
      $object_results = $solr_results['response']['docs'];
    }

    return $object_results;
}


