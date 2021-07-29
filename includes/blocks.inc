<?php

/**
 * @file
 * Contains all blocks related functionality
 */

// see https://api.drupal.org/api/examples/block_example%21block_example.module/7.x-1.x

global $weeks_ago;
$GLOBALS['weeks_ago'] = 25;
define ('OPENSKYDORA_USAGE_WEEKS_AGO', $GLOBALS['weeks_ago']);

/**
 * these "info" definitions are necessary to get the blocks on the
 * blocks admin UI page
 */
function openskydora_block_info() {
    $blocks = array();

    $blocks['openskydora_browse_collection'] = array (
        'info' => t('Openskydora Collection PID Search'),
    );

    $blocks ['openskydora_collection_info'] = array (
        'info' => t('Openskydora search collection information'),
    );

    $blocks ['openskydora_simple_search'] = array (
        'info' => t('OpenSkydora simple search'),
    );
    $blocks ['openskydora_advanced_search'] = array (
        'info' => t('OpenSkydora advanced search'),
    );

    $blocks ['openskydora_collection_data'] = array (
        'info' => t('OpenSkydora collection select data'),
    );

    return $blocks;
}

/**
 * implements hook_block_view
 *
 * openskydora Blocks
 */
function openskydora_block_view($delta = '') {
    $block = array();
    //    $pid = openskydora_get_pid_from_request();
    switch($delta) {
        
    case 'openskydora_browse_collection':
        $block['subject'] = t ('Collection Search for Browse view');
        $d_form = drupal_get_form('openskydora_browse_collection_search_form');
        $block['content'] = drupal_render($d_form);
        break;
            
    case 'openskydora_collection_info':
        $block['subject'] = '';
        $block['content'] = openskydora_search_collection_info();
        break;

    case 'openskydora_search_collection_search':
        $block['subject'] = t ('Opensky Collection Search title');
        $dform = drupal_get_form('openskydora_search_collection_search_form');
        $block['content'] = drupal_render($dform);
        break;

    case 'openskydora_advanced_search':
        $block['subject'] = t('OpenSky advanced search');
        $d_form = drupal_get_form('openskydora_advanced_search_form');
        $block['content'] = drupal_render($d_form);
        break;

    case 'openskydora_simple_search':
        $block['subject'] = t('OpenSky simple search');
        $form = drupal_get_form('openskydora_simple_search_form');
        $block['content'] = drupal_render($form);
        break;
        
    case 'openskydora_collection_data':
        $block['subject'] = t('Opensky collection picker data');
        $block['content'] = get_collection_select_options_js();
        break;

    case 'openskydora_trending_items':
        $block['subject'] = t('Opensky trending items');
        $block['content'] = get_trending_items_content();
        break;
        
    case 'openskydora_trending_images':
        $block['subject'] = t('Opensky trending images');
        $block['content'] = get_trending_images_content();
        break;
        
    case 'openskydora_trending_downloads':
        $block['subject'] = t('Opensky trending downloads');
        $block['content'] = get_trending_downloads_content();
        break;

    case 'openskydora_new_archival_items':
        $block['subject'] = t('New archives items');
        $block['content'] = get_new_archival_items_content();
        break;

    case 'openskydora_trending_collections':
        $block['subject'] = t('OpenSky Trending Collections');
        $block['content'] = get_trending_collections_content();
        break;
    }    
    return $block;
}

function get_new_archival_items_content() {
    module_load_include('inc', 'openskydora', 'includes/usage_stats');
    //$weeks_ago = 25;
    $weeks_ago = $GLOBALS['weeks_ago'];
    $count = 3;

    // archives version
    // $args = array ('RELS_EXT_hasModel_uri_t'=>'archives');

    // stage version
    $args = array ('RELS_EXT_hasModel_uri_t'=>'islandora');

    $new_items= openskydora_get_new_items($count, $weeks_ago, $args);

    return theme ('openskydora_new_items', array(
        'new_items' => $new_items,
        'title' => 'New in the Archives',
        'weeks_ago' => $GLOBALS['weeks_ago'],
    ));
}

function get_trending_collections_content() {
    module_load_include('inc', 'openskydora', 'includes/usage_stats');
    //$weeks_ago = 25;
    $weeks_ago = $GLOBALS['weeks_ago'];
    $items_count = 5;

    $most_viewed_collections = openskydora_get_most_viewed_collections(
        $items_count, $weeks_ago);

   // most_viewed_collections is simply coll_pid -> count
    $trending_collections = array();
    foreach ($most_viewed_collections as $coll_pid=>$views) {
        $pid = str_replace ('info:fedora/', '', $coll_pid);
        $obj = islandora_object_load ($pid);
        //        $label = $obj->label;
        if ($obj) {
            $trending_collections[$pid] = array(
                'label'=>$obj->label,
                'count'=>$views);
        }
    }
    return theme ('openskydora_trending_items', array(
        'trending_items' => $trending_collections,
        'title' => 'Trending Collections',
        'count' => $items_count,
        'weeks_ago' => $weeks_ago,
    ));
}

/* opensky_trending_items theme wants
   - $title ("trending items");
   - $trending_objects (array ('pid'=>array($count, $obj)))
   - $weeks_ago (5)
*/
function get_trending_downloads_content() {
    module_load_include('inc', 'openskydora', 'includes/usage_stats');
    //$weeks_ago = 25;
    $weeks_ago = $GLOBALS['weeks_ago'];
    $items_count = 3;

    // get all views and then look for images
    $most_downloaded_items = openskydora_get_most_downloaded($items_count, $weeks_ago);

    $trending_downloads = array();
    foreach($most_downloaded_items as $item) {

        $trending_downloads[$item->pid] = array(
            'count'=>$item->downloads,
            'label'=>$item->label
        );
    }

    return theme('openskydora_trending_items', array(
        'trending_items' => $trending_downloads,
        'title' => 'Trending Downloads',
        'count' => $items_count,
        'weeks_ago' => $weeks_ago,
    ));
}

/* opensky_trending_items theme wants
   - $title ("trending items");
   - $trending_objects (array ('pid'=>array($count, $obj)))
   - $weeks_ago (25)
*/
function get_trending_images_content() {
    module_load_include('inc', 'openskydora', 'includes/usage_stats');
    //    $weeks_ago = 5;
    $weeks_ago = $GLOBALS['weeks_ago'];
    $items_count = 3;

    // get all views and then look for images
    $most_viewed_items = openskydora_get_most_viewed(10000, $weeks_ago);

    $trending_images = array();
    foreach($most_viewed_items as $pid=>$item) {
        if (count($trending_images) >= $items_count) {
            break;
        }

        $obj = islandora_object_load($pid);
        if (in_array('islandora:sp_basic_image', $obj->models) ||
            in_array('islandora:sp_large_image_cmodel', $obj->models)) {

                $trending_images[$pid] = array(
                    'count'=>$item->views,
                    'label'=>$obj->label
                );
            }
    }
    
    return theme('openskydora_trending_items', array(
        'trending_items' => $trending_images,
        'title' => 'Trending Images',
        'count' => $items_count,
        'weeks_ago' => $weeks_ago,
    ));
}

/* 
   most recently viewed items
   opensky_trending_items theme wants
       - $title ("trending items");
       - $trending_objects (array ('pid'=>array($count, $obj)))
       - $weeks_ago (25)
*/
function get_trending_items_content() {
    module_load_include('inc', 'openskydora', 'includes/usage_stats');
    //$weeks_ago = 5;
    $weeks_ago = $GLOBALS['weeks_ago'];
    $items_count = 3;
    $exclude = array('opensky:root');
    $most_viewed_items = openskydora_get_most_viewed_items(
        $items_count,
        $weeks_ago,
        $exclude);

    $trending_items = array();
    foreach($most_viewed_items as $pid=>$item) {
        $trending_items[$pid] = array(
            'count'=>$item->views,
            'label'=>$item->obj->label
        );
    }
    
    return theme('openskydora_trending_items', array(
        'trending_items' => $trending_items,
        'title' => 'Trending items',
        'count' => $items_count,
        'weeks_ago' => $weeks_ago,
    ));
}

function get_most_viewed_items_content() {
    module_load_include('inc', 'openskydora', 'includes/usage_stats');
    //$most_viewed_items_weeks_ago = 25;
    $weeks_ago = $GLOBALS['weeks_ago'];
    $most_viewed_items_count = 5;
	$exclude = array('opensky:root');
    $most_viewed_items = openskydora_get_most_viewed_items(
        $most_viewed_items_count,
        $most_viewed_items_weeks_ago,
        $exclude);
    
    $output = theme('openskydora_most_viewed_items', array(
        'most_viewed_items_weeks_ago' => $most_viewed_items_weeks_ago,
        'most_viewed_items_count' => $most_viewed_items_count,
        'most_viewed_items' => $most_viewed_items,
    ));
   
    return $output;
}
/**
 * Generate javascript data for the collections select in simple and advanced search.
 * Show the current collection (if any), then 'All Collections' and then
 * all children (if any) of current collection (if any).
 *
 * see openskydora_collection_picker_data theme
 *
 * @return string
 *   NOTE: this block must be rendered as a region block for the request_uri
 *   be current. If we try to just display the block directly from template
 *   request_uri will be for PREVIOUS page and we won't get current collection
 *
 */
function get_collection_select_options_js () {

    $parent_pid = 'opensky:root';
    if (isset ($_GET['collection'])) {
        $parent_pid =  $_GET['collection'];
    } else {
        $pid = openskydora_get_pid_from_request();
        if ($pid && openskydora_is_collection_pid($pid)) {
            $parent_pid = $pid;
        }
    }

    $option_data = array ();
     if ($parent_pid != 'opensky:root') {
         $dc_array = openskydora_get_dc_array($parent_pid);
         if ($dc_array != null) {
             $title = html_entity_decode($dc_array['dc:title']['value']);
             $option_data[$parent_pid] = openskydora_truncate($title);
         }
     }

    $option_data[''] = 'All Collections';

    if (isset ($parent_pid)) {
        $children_info = openskydora_get_pid_subcollection_infos($parent_pid);
        foreach ($children_info as $child) {
            $option_data[$child['pid']] = openskydora_truncate($child['title']);
        }
    }

    return theme('openskydora_collection_picker_data', array(
        'option_data' => $option_data,
    ));
}

/* ---------------
   Populate a block that shows collection info and options for
   search
*/
function openskydora_search_collection_info() {
    $output = '';
    $collection = 'unknown';
    $params = $_GET;
//    dpm('openskydora_search_collection_info');
//    dpm($params);

    // is there a search term??
    $query = openskydora_get_search_term_from_request();
//    dpm('query: '.$query .' ('. strlen($query) .')');

    if (isset($params['collection'])) {
        $collection = $params['collection'];

        $title = $description = '';
        $dc_array = openskydora_get_dc_array($collection);
        if ($dc_array != null) {
            $title = $dc_array['dc:title']['value'];
            $description = $dc_array['dc:description']['value'];
        }
        $output = theme('openskydora_search_collection_info', array(
            'collection_pid' => $collection,
            'collection_title' => $title,
            'collection_description' => $description
        ));

    }
    return $output;
}

/**
 * Opensky item collection search form.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 *
 * @return array
 *   An associative array containing the fully built form structure.
 */
// function openskydora_search_collection_search_form($form, &$form_state) {

function openskydora_search_collection_search_form() {
    $collection_pid = '';
    if (isset ($_GET['collection'])) {
        $collection_pid = $_GET['collection'];
    }
    return get_openskydora_search_collection_form($collection_pid);
}


/**
 * Islandora Solr collection search form submit callback.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 */

function openskydora_search_collection_search_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_solr', 'includes/utilities');
  $search_string =
      islandora_solr_replace_slashes($form_state['values']['openskydora_collection_search_query']);

  $query = array(
      'type'=>'dismax',
      'collection'=>$form_state['values']['collection']
  );

  $form_state['redirect'] = array(
    ISLANDORA_SOLR_SEARCH_PATH . "/$search_string",
    array(
      'query' => $query,
    ),
  );
}

/**
 * Provides search form for collection BROWSE pages - used when there are
 * subcollections (as opposed to items) in the collection.
 */
function openskydora_browse_collection_search_form() {
    return get_openskydora_search_collection_form(openskydora_get_pid_from_request());
}

/**
 * Create form for "search this collection"
 */
function get_openskydora_search_collection_form($collection_pid) {
    $form = array(
        '#type' => 'container',
        '#attributes' => array(
            'class' => array(
                'container-inline',
            ),
        ),
    );

    $form['openskydora_collection_search_query'] = array(
        '#size' => '30',
        '#type' => 'textfield',
        '#title' => '<none>',
        // @todo Should this be the searched value?
        '#default_value' => '',
    );

    $form['collection'] = array (
        '#type' => 'hidden',
        '#default_value' => $collection_pid,
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('search'),
    );

    return $form;
}

/**
 * Islandora Solr collection search form submit callback.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 */
function openskydora_browse_collection_search_form_submit($form, &$form_state) {
    module_load_include('inc', 'islandora_solr', 'includes/utilities');
    $search_string = islandora_solr_replace_slashes($form_state['values']['openskydora_collection_search_query']);

    $query = array(
        'type'=>'dismax',
      'collection'=>$form_state['values']['collection']
    );

    $form_state['redirect'] = array(
        ISLANDORA_SOLR_SEARCH_PATH . "/$search_string",
        array(
            'query' => $query,
        ),
    );
}

//--------------------------
/**
 * Islandora Solr simple search form.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 *
 * @return array
 *   An associative array containing the fully built form structure.
 */

function openskydora_simple_search_form($form, &$form_state) {

    $form['simple'] = array(
        '#type' => 'container',
        '#attributes' => array(
            'class' => array(
                'container-inline',
            ),
        ),
    );

    //---------------
    $form['simple']['collection'] = array(
        '#title' => t('Search in'),
        '#type' => 'select',
        '#default_value' => '',
        '#options' => array (),
        '#validated' => 'TRUE',
    );
    // ------------------------


    $form['simple']["openskydora_simple_search_query"] = array(
        '#size' => '46',
        '#type' => 'textfield',
        '#title' => 'for',
        // @todo Should this be the searched value?
        '#default_value' => '',
    );
    $form['simple']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('  Go  '),
    );
    return $form;
}

/**
 * Islandora Solr simple search form submit callback.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 */
function openskydora_simple_search_form_submit($form, &$form_state) {
    module_load_include('inc', 'islandora_solr', 'includes/utilities');
    $search_string = islandora_solr_replace_slashes($form_state['values']['openskydora_simple_search_query']);

    $query = array('type' => 'dismax');


    if (isset ($form_state['values']['collection'])) {
        $col = $form_state['values']['collection'];
        if ($col) {
            $query['collection'] = $col;
        }
    }

    $form_state['redirect'] = array(
        ISLANDORA_SOLR_SEARCH_PATH . "/$search_string",
        array(
            'query' => $query,
        ),
    );
}

/**
 * Islandora Solr advanced search block form.
 *
 * Check form states:
 * 1: Form update using AJAX.
 * 2: Populate with current query on search results page.
 * 3: Anywhere else: empty form.
 *
 * @link
 * http://drupal.stackexchange.com/questions/14855/how-do-i-dynamically-fill-a-textfield-with-ajax/16576#16576
 * Some exam\
ple AJAX. @endlink
*
 * @global IslandoraSolrQueryProcessor $_islandora_solr_queryclass
 *   The IslandoraSolrQueryProcessor object which includes the current query
 *   settings and the raw Solr results.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 *
 * @return array
 *   An associative array containing the fully built form structure.
 */
function openskydora_advanced_search_form($form, &$form_state) {
    module_load_include('inc', 'islandora_solr', 'includes/utilities');
    global $_islandora_solr_queryclass;

    // 1: Form update using AJAX.
    if (isset($form_state['triggering_element'])) {

        unset($form_state['input']['terms']);
        // Check for add.
        if ($form_state['triggering_element']['#value'] == '+') {
            $form_state['values']['terms'][] = array();
        }
        // Check for remove.
        elseif ($form_state['triggering_element']['#value'] == '-') {
            $field = $form_state['triggering_element']['#field'];
            array_splice($form_state['values']['terms'], $field, 1);
        }

        $values = $form_state['values'];
    }
    // 2: Populate with current query on search results page.
    elseif (islandora_solr_results_page($_islandora_solr_queryclass) == TRUE && 
            !isset($_islandora_solr_queryclass->internalSolrParams['type'])) {

        // Get current query.
        $query = $_islandora_solr_queryclass->solrQuery;

        $values['terms'] = array();

        $query_explode = preg_split(ISLANDORA_SOLR_QUERY_SPLIT_REGEX, $query);

        // Break up the solr query to populate the advanced search form.
        $i = 0;
        foreach ($query_explode as $key => $value) {
            $term = array();
            // Check for first colon to split the string.
            if (strpos($value, ':') != FALSE) {
                // Split the filter into field and value.
                $value_split = preg_split(ISLANDORA_SOLR_QUERY_FIELD_VALUE_SPLIT_REGEX, $value, 2);

                $values['terms'][$i]['field'] = stripslashes($value_split[0]);

                // Second part of the split is the query value (or first part of
                // it).
                $value_split[1] = str_replace(array('(', ')'), '', $value_split[1]);

                $values['terms'][$i]['search'] = $value_split[1];

            }
            // If the string does not include a colon or AND/OR/NOT, then it is
            // a
            // part of the query value.
            elseif (!preg_match('/(AND|OR|NOT)/', $value, $matches)) {
                // Trim brackets.
                $value = str_replace(array('(', ')'), '', $value);

                if (isset($values['terms'][$i]['search'])) {
                    // Append to search string.
                    $values['terms'][$i]['search'] .= ' ' . $value;
                }
                else {
                    // Search field is not set, so create new search value.
                    $values['terms'][$i]['search'] = $value;
                }
            }
            // If it matches AND/OR/NOT, then we have the boolean operator.
            else {
                $values['terms'][$i]['boolean'] = $value;

                // XXX: Something about only incrementing here seems... Wrong?
                $i++;
            }
        }
    }
    // 3: Anywhere else: empty form.
    else {
        // Need at least one term to draw the search box.
        $values = array(
            'terms' => array(''),
        );
    }

    $terms = array(
        '#type' => 'markup',
        '#prefix' => '<div id="openskydora-advanced-terms">',
        '#suffix' => '</div>',
        '#tree' => TRUE,
    );
    foreach ($values['terms'] as $i => $value) {
        $term = array(
            '#tree' => TRUE,
            '#prefix' => '<div>',
            '#suffix' => '</div>',
        );
        $term['field'] = array(
            '#title' => t('Field'),
            '#type' => 'select',
            '#default_value' => isset($value['field']) ? $value['field'] : 'dc.title',
            '#options' => islandora_solr_get_fields('search_fields'),
        );
        $term['search'] = array(
            '#title' => t('Search terms'),
            '#type' => 'textfield',
            '#size' => 20,
            '#default_value' => isset($value['search']) ? $value['search'] : '',
        );

        // Used for when the user presses enter on the search field.
        $term['hidden_submit'] = array(
            '#type' => 'submit',
            '#value' => t('Search'),
            '#attributes' => array('style' => 'visibility:hidden;position:fixed;top:-1000px;right:-1000px;'),
        );
        $term['add'] = array(
            '#type' => 'button',
            '#value' => '+',
            '#attributes' => array('title' => t('Add field')),
            '#name' => 'add-field-' . $i,
            '#ajax' => array(
                'callback' => '_openskydora_advanced_search_terms',
                'wrapper' => 'openskydora-advanced-terms',
                'method' => 'replace',
                'effect' => 'fade',
                'progress' => array('type' => 'none'),
            ),
        );
        if (count($values['terms']) > 1) {
            $term['remove'] = array(
                '#type' => 'button',
                '#field' => $i,
                '#value' => '-',
                '#attributes' => array('title' => t('Remove field')),
                '#name' => 'remove-field-' . $i,
                '#ajax' => array(
                    'callback' => '_openskydora_advanced_search_terms',
                    'wrapper' => 'openskydora-advanced-terms',
                    'method' => 'replace',
                    'effect' => 'fade',
                    'progress' => array('type' => 'none'),
                ),
            );
            if ((variable_get('openskydora_search_boolean', 'user') == 'user') && ((count($values['terms']) - 1) != $i)) {
                $term['boolean'] = array(
                    '#type' => 'select',
                    '#prefix' => '<div>',
                    '#suffix' => '</div>',
                    '#default_value' => isset($value['boolean']) ? $value['boolean'] : 'AND',
                    '#options' => array(
                        'AND' => 'AND',
                        'OR' => 'OR',
                        'NOT' => 'NOT',
                    ),
                );
            }
        }

        $terms[] = $term;
    }

    // Add terms.
    $form['terms'] = $terms;

    // ------------------------
    // collections options supplied by openskydora_collection_select_options block,
    // and inserted into collection select via javascript (see custom.js in openskydora theme)
    // All because we can't get current collection from this function: it is evaluated apparently
    // too early -> request_uri is the PREVIOUS page, not this one!?

    $form['collection'] = array(
        '#title' => t('Collection'),
        '#type' => 'select',
        '#default_value' => '',
        '#options' => array (),
        '#validated' => 'TRUE',
    );
    // ------------------------

    // Add controls.
    $form['controls'] = array(
        '#type' => 'markup',
        '#prefix' => '<div class="islandora-solr-advanced-controls">',
        '#suffix' => '</div>',
    );
    $form['controls']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Search'),
    );
    return $form;
}

function openskydora_form_openskydora_advanced_search_form_alter(&$form, &$form_state, $form_id) {
    /*
    // No help for getting current request
    dpm ('openskydora_form_advanced_search_form_alter!! - ' . $form_id);
    dpm('request_uri: '.request_uri());
    dpm($_GET);
    */
}

/**
 * AJAX callback for advanced search form.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 *
 * @return array
 *   An associative array containing the fully built form element.
 *
     * @see openskydora_advanced_search_form()
 */
function _openskydora_advanced_search_terms($form, &$form_state) {
    // dpm('_openskydora_advanced_search_terms');
    // dpm($form_state);
    return $form['terms'];
}


/**
 * Islandora Solr advanced search form submit callback.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 *
 * @see openskydora_advanced_search_form()
 */
function openskydora_advanced_search_form_submit($form, &$form_state) {
    module_load_include('inc', 'islandora_solr', 'includes/utilities');
    // Collect query values.
    $query_array = array();
    foreach ($form_state['values']['terms'] as $term) {
        $field = $term['field'];

        $search = trim($term['search']);
        $search = islandora_solr_replace_slashes($search);


        $boolean = (isset($term['boolean'])) ? $term['boolean'] : variable_get('openskydora_search_boolean', 'user');
        $field = islandora_solr_lesser_escape($field);
        // Add query.
        if (!empty($search)) {
            $query_array[] = array(
                'search' => "$field:($search)",
                'boolean' => $boolean,
            );
        }
    }

    // Create query.
    $query = '';
    $i = 0;
    foreach ($query_array as $term) {
        $query .= $term['search'];
        if (count($query_array) - 1 != $i) {
            $query .= ' ' . $term['boolean'] . ' ';
        }
        $i++;
    }

    // --------------------
    $params = array();

    if (isset ($form_state['values']['collection'])) {
        $col = $form_state['values']['collection'];
        if ($col) {
            $params['collection'] = $col;
//            $params['type'] = 'dismax';
        }
    }

    // -----------------------

    // Check if query is empty.
    if (empty($query) && empty($params)) {
        $query = '*:*';
    }

    // Navigate to results page.
    $form_state['redirect'] = array(
        ISLANDORA_SOLR_SEARCH_PATH . "/$query",
        array (
            'query' => $params,
        ),
    );
}
