<?php

function jlo_collections_get_collection ($pid) {
    $islandora_object = openskydora_get_fedora_object($pid);
    //dsm('islandora_object');
    //dsm($islandora_object);
    

    try {
        $dc = $islandora_object['DC']->content;
        $dc_object = DublinCore::importFromXMLString($dc);
        $dc_array = openskydora_get_dc_array($pid);
    }
    catch (Exception $e) {
        drupal_set_message(t('Error retrieving object %s %t',
                             array(
                                 '%s' => $islandora_object->id,
                                 '%t' => $e->getMessage())),
                           'error',
                           FALSE);
    }
    $title = $dc_array['dc:title']['value'];
    $description = $dc_array['dc:description']['value'];
    
    $img_url = url("islandora/objects/$pid/datastream/TN");
    $object_url = 'islandora/object/' . $pid;

    # haven't been able to make this work
    $thumb_url = url("islandora/objects/$pid/datastream/TN/view");
    $thumbnail = theme('image', array (
        'title'=>$title,
        'path'=>$thumb_url,
    ));
    
    $jlo_collection = array (
        'description'=>str_replace("'", "&apos;",$description),
        'object_url'=>'/islandora/object/' . $pid,
        'pid'=>$pid,
#        'thumbnail'=>$thumbnail,
        'landing_page'=>'/islandora/object/' . $pid,
        'title'=>str_replace("'", "&apos;", $title),
    );

    return $jlo_collection;
}

function jlo_collections_get_collections (){

    $root_pid = ROOT_PID;
    $collection_pids = openskydora_get_subcollections_recursive($root_pid);
    $tree = openskydora_get_collection_tree();

    foreach($collection_pids as $pid) {

        $jlo_collection = jlo_collections_get_collection($pid);
        $jlo_collection['children'] = $tree[$pid];
        $jlo_collections[$pid] = $jlo_collection;
    }
    return $jlo_collections;
}
