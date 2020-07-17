<?php

/**
 * @file
 * Convert mods data into JSON for citeproc-js.
 */

/**
 * Perform XPath queries to extract CSL info from provided $mods.
 *
 * This function is called after islandora_scholar module converter.inc
 * has built the $output array, which is modified with openskydora-specific
 * attributes.
 * @param array $output
 *   An associative array mapping CSL variable names to values we have
 *   extracted for them from the provided MODS record.
 *
 * @param SimpleXMLElement $mods
 *   Either a DOMNode or a string representing a MODS record.
 *
 */
function openskydora_convert_mods_to_citeproc_jsons_alter(array &$output, SimpleXMLElement $mods) {

  if ($mods instanceof SimpleXMLElement) {
      add_mods_namespace($mods);

      $gtype = openskydora_convert_mods_to_citeproc_json_genre($mods);
      switch (trim($gtype)) {
          case 'paper-conference': {
              $container_title = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:name[@type="conference"]/mods:namePart');
              break;
          }
          default: {
              $container_title = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:relatedItem[@type="host"]/mods:titleInfo[not(@type)]/mods:title');
          }
      }

      // replace selected values with openskydoravalues
      $output['container-title'] = $container_title;
      $output['volume'] = (int)convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:relatedItem[@type="host"]/mods:part/mods:detail[@type="volume"]/mods:number');
      $output['issue'] = (int)convert_mods_to_citeproc_json_query($mods, variable_get('islandora_scholar_xpaths_issue', '//mods:mods[1]/mods:part/mods:detail[@type="issue"]/mods:number'));
      $output['publisher'] = openskydora_convert_mods_to_citeproc_json_publisher($mods);
      $output['publisher-place'] = openskydora_convert_mods_to_citeproc_json_publisher_place($mods);
      $output['URL'] = convert_mods_to_citeproc_json_query($mods, variable_get('islandora_scholar_xpaths_URL', '//mods:mods[1]/mods:location/mods:url'));
      $output['page'] = openskydora_convert_mods_to_citeproc_json_page($mods);
      $output['number'] = openskydora_convert_mods_to_citeproc_json_number($mods);
      $output['number-of-pages'] = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:part/mods:extent[@unit="pages"]/mods:total');
      $output['type'] = openskydora_convert_mods_to_citeproc_json_genre($mods);

      // special handling for primary corporate authors
      $primary_corporate_author = openskydora_get_primary_corporate_author($mods);

      if ($primary_corporate_author) {
          $output['author'] = $primary_corporate_author;
      }
  }
}

/**
rewrite the json for primary_corporate authors so it can be processed
downstream (see CiteProc-> csl_name())

takes the first if muliple primary_corporate_authors are found

 */
function openskydora_get_primary_corporate_author (SimpleXMLElement $mods) {

    $pm_names = $mods->xpath("//mods:name[@type='corporate'][@usage='primary']");
    
    if (count($pm_names) > 0) {
        
        $namePart = (string)$pm_names[0]->namePart;
        return array (
            array (
                'corporate' => $namePart,
            ));
    }
}


/**
 * Gets the page property for the Citation.
 *
 * @param SimpleXMLElement $mods
 *   A MODS document.
 *
 * @return string
 *   The page property for the Citation.
 */
function openskydora_convert_mods_to_citeproc_json_page(SimpleXMLElement $mods) {
    $output = NULL;
    $article_number = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:relatedItem[@type="host"]/mods:part/mods:detail[@type="citation/article"]/mods:number');
    //    if (isset($article_number)) {
    //        $output = $article_number;
    //    } else {
    $pages = $mods->xpath("//mods:mods[1]/mods:relatedItem/mods:part/mods:extent[@unit='page']");
    // Note: "pages" is correct, but Zotero uses "page".
    if (empty($pages)) {
        $pages = $mods->xpath("//mods:mods[1]/mods:relatedItem/mods:part/mods:extent[@unit='pages']");
    }
    if (isset($pages[0]->list) || isset($pages[0]->start)) {
        if (!empty($pages[0]->list)) {
            $output = (string) $pages[0]->list;
        } elseif (!empty($pages[0]->start)) {
            $output = (string) $pages[0]->start;
            if (!empty($pages[0]->end)) {
                $output .= "-" . $pages[0]->end;
            }
        }
    } elseif (isset($article_number)) {
        $output = $article_number;
    } elseif (!empty($pages[0]->total)) {
        $output = (string) $pages[0]->total . " pp";
    }
    //    }
    return $output;
}

function openskydora_convert_mods_to_citeproc_json_number(SimpleXMLElement $mods) {
    $genre = trim(convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:genre'));
    if (($genre == 'report') or ($genre == 'technical report') or ($genre == 'workshop report') or ($genre == 'progress report') or ($genre == 'white paper')) {
#      $output = $mods->xpath("//mods:mods[1]/mods:relatedItem[@type='series']/mods:part[not(@type)]/mods:detail[@type='report']/mods:number");
        $output = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:relatedItem[@type="series"]/mods:part[not(@type)]/mods:detail[@type="report"]/mods:number');
    }
    else {
        $output = $mods->xpath("//mods:mods[1]/mods:relatedItem[@type='series']/mods:titleInfo/mods:partNumber");
    }
    return !empty($output) ? $output : NULL;
}


/**
 * Gets the publisher property for the Citation.
 *
 * @param SimpleXMLElement $mods
 *   A MODS document.
 *
 * @return string
 *   The publisher property for the Citation.
 */
function openskydora_convert_mods_to_citeproc_json_publisher(SimpleXMLElement $mods) {

    $gtype = openskydora_convert_mods_to_citeproc_json_genre($mods);
    switch (trim($gtype)) {
        case 'paper-conference': {
            $publisher = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:place/mods:placeTerm[@type="text"]');
            $publisher2 = explode ("-", convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:place/mods:placeTerm[@authority="iso3166"]'));
            $publisher3 = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:place/mods:placeTerm[@authority="marccountry"]');
            if (array_key_exists('1', $publisher2)) {
                $publisher = $publisher . ', ' . $publisher2[1];
            }
            if (isset($publisher3)) {
                $publisher = $publisher . ', ' . $publisher3;
            }
            return $publisher;
        }
        default: {
            $publisher = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:publisher');
            return $publisher;
        }
    }
}

/**
 * Gets the publisher-place property for the Citation.
 *
 * @param SimpleXMLElement $mods
 *   A MODS document.
 *
 * @return string
 *   The publisher-place property for the Citation.
 */
function openskydora_convert_mods_to_citeproc_json_publisher_place(SimpleXMLElement $mods) {

    $gtype = openskydora_convert_mods_to_citeproc_json_genre($mods);
    switch (trim($gtype)) {
        case 'paper-conference': {
            $publisher_place = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:name[@type="corporate"]/mods:namePart');
            $article_number = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:relatedItem[@type="host"]/mods:part/mods:detail[@type="citation/article"]/mods:number');
            return $publisher_place;
        }
        default: {
            $publisher_place = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:place/mods:placeTerm');
            return $publisher_place;
        }
    }
}


// ---------- Changed Functions ------------------

/**
 * Convert the given MODS genre to JSON repersentation.
 *
 * @param mixed $mods
 *   A MODS document.
 *
 * @return string
 *   The JSON repersentation of the MODS genre.
 */
function openskydora_convert_mods_to_citeproc_json_genre($mods) {
    $genre = convert_mods_to_citeproc_json_query($mods, variable_get('islandora_scholar_xpaths_genre', '//mods:mods[1]/mods:genre'));
    switch (trim($genre)) {
        case 'journal article':
            return 'article-journal';

        case 'chapter':
        case 'book chapter':
        case 'book section':
            return 'chapter';

        case 'book':
            return 'book';

        case 'technical report':
        case 'progress report':
        case 'workshop report':
        case 'white paper':
            return 'report';

        case 'conference material':
            return 'paper-conference';

        case 'proceedings article':
            return 'paper-conference';


        default:
            return 'article-journal';
    }
}

// ============= Replicated Functions =====================
/**
 * Gets the first result of the provided $path.
 *
 * @param SimpleXMLElement $mods
 *   A MODS document.
 * @param string $path
 *   An XPath.
 *
 * @return string
 *   The first results of the query if found NULL otherwise.
 */
function openskydora_convert_mods_to_citeproc_json_query(SimpleXMLElement $mods, $path) {
    $property = $mods->xpath($path);
    return !empty($property) ? (string) $property[0] : NULL;
}

/**
 * Gets the publisher property for the Citation.
 *
 * @param SimpleXMLElement $mods
 *   A MODS document.
 *
 * @return string
 *   The publisher property for the Citation.
 */
function convert_mods_to_citeproc_json_publisher(SimpleXMLElement $mods) {

    $gtype = openskydora_convert_mods_to_citeproc_json_genre($mods);
    switch (trim($gtype)) {
        case 'paper-conference': {
            $publisher = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:place/mods:placeTerm[@type="text"]');
            $publisher2 = explode ("-", convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:place/mods:placeTerm[@authority="iso3166"]'));
            $publisher3 = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:place/mods:placeTerm[@authority="marccountry"]');
            if (array_key_exists('1', $publisher2)) {
                $publisher = $publisher . ', ' . $publisher2[1];
            }
            if (isset($publisher3)) {
                $publisher = $publisher . ', ' . $publisher3;
            }
            return $publisher;
        }
        default: {
            $publisher = convert_mods_to_citeproc_json_query($mods, '//mods:mods[1]/mods:originInfo/mods:publisher');
            return $publisher;
        }
    }
}

// ========================= KLUDGES =================

if (!function_exists ('variable_get')) {
    function variable_get($pathname, $default_value) {
        print ("variable_get()");
        return $default_value;
    }

}
