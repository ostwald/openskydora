<?php
/**
 * implemnts HOOK_islandora_breadcrumbs_alter
 * hook to custom format the breadcrumbs (path, search_term and facets)
 */
function openskydora_islandora_breadcrumbs_alter(&$data, $context) {

    $search_label_breadcrumb = '<span class="search-breadcrumb">Search</span>';
    $filter_label_breadcrumb = '<span class="filter-breadcrumb">Filter</span>';

    // pull info from request
    $query = @$_GET['q'];
    $filters = @$_GET['f'];
    $collection = @$_GET['collection'];

    if (strpos(current_path(), '/search/')) {
        $parts = explode('/', current_path());
        $search_term = end($parts);
    }

    if (isset($search_term) && strlen(trim($search_term)) > 0) {
        $search_data = $data[1];
        if (count($data) > 2) {
            $filter_data = array_slice($data, 2);
        }
    } else if ($filters) {
        $filter_data = array_slice($data, 1);
    }

    // dsm("search_term: $search_term, filters: " . @count($filters) . ", collection: $collection");
    
    $path_crumbs = array();
    $search_crumbs = array();
    $filter_crumbs = array();
    
    if (isset($search_data) || isset($filter_data)) {

        if (isset($search_data)) {
            $search_crumbs[] = $search_label_breadcrumb;
            $search_crumbs[] = $search_data;
        }
        if (isset($filter_data)) {
            $filter_crumbs[] = $filter_label_breadcrumb;
            foreach ($filter_data as $filter_term) {
                $filter_crumbs[] = $filter_term;
            }
        }
        // when there is search or filter data, the only path data is
        // the first element
        $data = array($data[0]);
    }

    if ($collection) {
        module_load_include('inc', 'islandora', 'includes/breadcrumb');
        $obj = openskydora_get_fedora_object($collection);
        if ($obj) {
            $path_crumbs = islandora_get_breadcrumbs_recursive($obj->id, $obj->repository);
        } else {
            $path_crumbs = array ();
        }

        // PATH_BREADCRUMBS - anscestors in order from root down to parent
        if (!isset($search_data) & !isset($filter_data)) {
            
            // pop the parent and strip the link, leaving collection name
            $popped = array_pop($path_crumbs);
            preg_match ('/<.*>(.*)<.*/', $popped, $match);
            if ($match) {
                $path_crumbs[] = $match[1];
            }
            else {
                // current collection unknown!!?
                $path_crumbs[] = '_collection_name_here_';
            }
        }
    }
    else {
        $pid = openskydora_get_pid_from_request();
        if (!$pid) {
            $pid = variable_get('islandora_repository_pid');
        }

        if (openskydora_is_collection_pid ($pid)) {
            $dc_array = openskydora_get_dc_array($pid);                                          
            $collection_name = $dc_array['dc:title']['value'];
            $data[] = $collection_name;
            // $data = array($data[0], $collection_name);
        }
        $path_crumbs = $data;
    }

    $data = array_merge($path_crumbs, $search_crumbs, $filter_crumbs);
    
    if (count($data) > 1) {
        $data[1] = str_replace('Islandora Repository', 'Collections', $data[1]);
    }
}

function get_parent_breadcrumb_for_pid ($pid) {
    $obj = openskydora_get_fedora_object($pid);
    module_load_include('inc', 'islandora', 'includes/metadata');
    module_load_include('inc', 'islandora', 'includes/utilities');
    $parent_collections = islandora_get_parents_from_rels_ext ($obj);    
    $parent = @$parent_collections[0];
    if (!$parent) {
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

// --------------------------------------------------


/**
 * implemnts HOOK_islandora_breadcrumbs_alter
 * hook to custom format the breadcrumbs (path, search_term and
 * facets)
 *
 * OLD VERSION - no longer used, but shows an ELIPSIS PATH for search
 * and filter cases
 */
function openskydora_islandora_breadcrumbs_elipsis(&$data, $context) {

    $search_label_breadcrumb = '<span class="search-breadcrumb">Search</span>';
    $filter_label_breadcrumb = '<span class="filter-breadcrumb">Filter</span>';

    // pull info from request
    $query = @$_GET['q'];
    $filters = @$_GET['f'];
    $collection = @$_GET['collection'];

    if (strpos(current_path(), '/search/')) {
        $parts = explode('/', current_path());
        $search_term = end($parts);
    }

    if (isset($search_term) && strlen(trim($search_term)) > 0) {
        $search_crumb = $data[1];
        if (count($data) > 2) {
            $filter_crumbs = array_slice($data, 2);
        }
    } else if ($filters) {
        $filter_crumbs = array_slice($data, 1);
    }

    // dsm("search_term: $search_term, filters: " . @count($filters) . ", collection: $collection");

    if (isset($search_crumb) || isset($filter_crumbs)) {

        // build a new breadcrumb structure
        $new_crumbs = array();
        $new_crumbs[] = $data[0];
            
        if ($collection) {
            $parent_breadcrumb = get_parent_breadcrumb_for_pid($collection);
            if ($parent_breadcrumb) {
                $new_crumbs[] = ' ... ';
            }
            $collection_breadcrumb = openskydora_collection_breadcrumb($_GET);
            $new_crumbs[] = $collection_breadcrumb;
        }
        if (isset($search_crumb)) {
            $new_crumbs[] = $search_label_breadcrumb;
            $new_crumbs[] = $search_crumb;
        }
        if (isset($filter_crumbs)) {
            $new_crumbs[] = $filter_label_breadcrumb;
            foreach ($filter_crumbs as $filter_crumb) {
                $new_crumbs[] = $filter_crumb;
            }
        }
        $data = $new_crumbs;
    }

    else if ($collection) {

        if (count($data) == 1) {
            // PATH_BREADCRUMBS - anscestors in order from root down to parent
            module_load_include('inc', 'islandora', 'includes/breadcrumb');
            $obj = openskydora_get_fedora_object($collection);
            $path_breadcrumbs = islandora_get_breadcrumbs_recursive($obj->id, $obj->repository);

            // pop the parent and strip the link, leaving collection name
            $popped = array_pop($path_breadcrumbs);
            preg_match ('/<.*>(.*)<.*/', $popped, $match);
            if ($match) {
                $path_breadcrumbs[] = $match[1];
            }
            else {
                // current collection unknown!!?
                $path_breadcrumbs[] = '_collection_name_here_';
            }
            
            $data = $path_breadcrumbs;
        } else {
            $dc_array = openskydora_get_dc_array($collection);
            $collection_name = $dc_array['dc:title']['value'];
            $data[] = $collection_name;            
        }
    }
    else {

        $pid = openskydora_get_pid_from_request();
        if (!$pid) {
            $pid = variable_get('islandora_repository_pid');
        }
        if (openskydora_is_collection_pid ($pid)) {
            $dc_array = openskydora_get_dc_array($pid);                                          
            $collection_name = $dc_array['dc:title']['value'];
            $data[] = $collection_name;
        }
        
    }
    
    if (count($data) > 1) {
        $data[1] = str_replace('Islandora Repository', 'Collections', $data[1]);
    }
}
