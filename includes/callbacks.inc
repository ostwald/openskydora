<?php
/**
 * Menu alter replacement callback for ds downloads.
 *
 * based on islandora_usage_stats_track_downloads
 * @param AbstractDatastream $datastream
 *   The datastream being downloaded.
 */
function openskydora_track_views_as_downloads(AbstractDatastream $datastream) {
    module_load_include('inc', 'islandora', 'includes/datastream');
    if ($datastream->id == 'PDF' || $datastream->id == 'OBJ') {
        module_load_include('inc', 'islandora_usage_stats', 'includes/utilities');
        islandora_usage_stats_log_datastream_download($datastream->parent->id, $datastream->id);
    }
    islandora_view_datastream($datastream);
}
