<?php

/**
 * hook to insert the collection link and x_link into the breadcrumbs (see
 * islandora_solr_search/includes/results->setBreadcrumbs()

For whatever evil reason, openskydora_islandora_breadcrumbs_alter gets called
TWICE. Therefore, we have to detect when we're called the second time and
politely refuse to do anything .....
 */
function openskydora_islandora_breadcrumbs_alter(&$data, $context) {
    // dsm ('openskydora_islandora_breadcrumbs_alter');
    // dsm ( ' current path: ' . current_path());
    // dsm ($_GET);
    /*
    if (isset ($_GET['islandora_solr_search_navigation'])) {
        dsm ("islandora_solr_search_navigation");
    }
    */
    $search_label_breadcrumb = '<span class="search-breadcrumb">Search</span>';
    $is_search = FALSE;
    if (isset ($_GET['f'])) {
        $is_search = TRUE;
    }
    else if (strpos(current_path(), '/search/')) {

        $parts = explode('/', current_path());
        $search_term = end($parts);
        $is_search = (strlen(trim($search_term)) > 0);

        if (strlen(trim($search_term)) > 0) {
            $is_search = TRUE;
        } else {
            $is_search = FALSE;
        }
    }

    $collection = null;
    // only make change if there is a collection param
    // unless the 'f' param is present, which signals facets/data range/etc
    if (!isset($_GET['collection'])) {
        //        dsm (' - COLLECTION - returning ...');

        if ($is_search) {
            array_splice ($data, 1, 0, $search_label_breadcrumb);
            //$data[0] = '<span class="search-breadcrumb">Search</span>';
        }

        // breadcrumbs look like this: <a href="/islandora">Islandora Repository</a>
        // Don't want 'Islandora Repository' to appear in breadcrumbs
        if (isset ($data[1])) {
            $data[1] = str_replace('Islandora Repository', 'OpenSky Collections', $data[1]);
        }
        return;
    } else {
        $collection = $_GET['collection'];
    }


    if (count($data) == 1) {
        // current bookmarks contain neither search or path

        // PATH_BREADCRUMBS - anscestors in order from root down to parent
        module_load_include('inc', 'islandora', 'includes/breadcrumb');
        //    module_load_include('inc', 'openskydora', 'includes/utilities');
        $obj = openskydora_get_fedora_object($collection);
        $path_breadcrumbs = islandora_get_breadcrumbs_recursive($obj->id, $obj->repository);
        array_pop($path_breadcrumbs);
        $data = $path_breadcrumbs;

    }
    
    if ($is_search && isset($collection)) {
        $collection_breadcrumb = openskydora_collection_breadcrumb($_GET);
        array_splice ($data, 1, 0, $collection_breadcrumb);

        /*
        try {
            $parent_breadcrumb = get_parent_for_pid($_GET['collection']);
            if ($parent_breadcrumb) {
                //dsm ("parent: $parent_breadcrumb");
                array_splice ($data, 1, 0, $parent_breadcrumb);
            }
        } catch (Exception $e) {
            echo 'Caught exception: ',  $e->getMessage(), "\n";
        }
        */
    }

    if ($is_search) {
        // dsm ("inserting SEARCH crumb");
        array_splice ($data, 1, 0, $search_label_breadcrumb);
        // $data[0] = '<span class="search-breadcrumb">Search</span>';
    }
    $data[1] = str_replace('Islandora Repository', 'OpenSky Collections', $data[1]);
}

function get_parent_for_pid ($pid) {
    $obj = openskydora_get_fedora_object($pid);
    module_load_include('inc', 'islandora', 'includes/metadata');
    module_load_include('inc', 'islandora', 'includes/utilities');
    $parent_collections = islandora_get_parents_from_rels_ext ($obj);    
    $parent = $parent_collections[0];
    if (!$parent) {
        dsm ("parent not found");
        return FALSE;
    }
    
    $query = array ('type' => 'dismax',
                    'collection' => $parent->id,
    );

    $attr = array (
        'href' => url(current_path(), array('query' => $query)),
        'class' => 'collection-crumb'
    );

    $breadcrumb = '<a' . drupal_attributes($attr) . '>' . check_plain($parent->label) . '</a>';
    return $breadcrumb;
}

/**
 * creates the collection breadcrumb, which is inserted into
 * breadcrumbs by openskydora_islandora_breadcrumbs_alter
 */
function openskydora_collection_breadcrumb($params) {
    $collection = $params['collection'];     
    $dc_array = openskydora_get_dc_array($collection);
    $filter_string = $dc_array['dc:title']['value'];
    
    // Remove filter array for collection breadcrumb link
    $query = array();
    
    $query_diff = $params;
    
    if (isset($query_diff['f'])) {
        unset($query_diff['f']);
    }
    
    // Set attributes variable.
    $attr = array();
    $attr['title'] = $filter_string;
    $attr['rel'] = 'nofollow';
    if (isset($exclude) && $exclude) {
        $attr['class'] = 'strikethrough';
    }
    // remove query from path if there is one
    // $attr['href'] = url(ISLANDORA_SOLR_SEARCH_PATH, array('query' => $query_diff));
    $attr['href'] = url('/islandora/search', array('query' => $query_diff));
        
    // Pull out filter (for x link).
    $query_x = array();  // query will be like current except it will not have collection param
    foreach ($params as $key => $value) {
        if ($key != 'collection' && $value == $collection) $query_x[$key] = $value;
    }

    $attr_x = array();
    $attr_x['title'] = t('Remove') . ' ' . $filter_string;
    $attr_x['rel'] = 'nofollow';
    $attr_x['href'] = url(current_path(), array('query' => $query_x));

    if (FALSE) {
        $breadcrumb = '<a' . drupal_attributes($attr) . '>' . check_plain($filter_string) . '</a>'
                    . '<span class="islandora-solr-breadcrumb-super collection-crumb"> <a' . drupal_attributes($attr_x) . '>('
                    . t('x') . ')</a></span>';
    } else {
        $breadcrumb = '<a' . drupal_attributes($attr) . '>' . check_plain($filter_string) . '</a>';
    }
    return $breadcrumb;
}
