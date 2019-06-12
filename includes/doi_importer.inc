<?php

/*
 * Implements hook_islandora_doi_content_type_info_alter
 *
 * swap out scholar's islandora_doi translator with our own
 * see swap out scholar's islandora_doi translator with our own
 */
function openskydora_islandora_doi_content_type_info_alter (array &$info) {
    $crossref_type = 'application/vnd.crossref.unixref+xml';
    $info[$crossref_type]['translator'] = 'openskydora_crossref_translator';
}

/**
 * Implements callback_islandora_doi_translator() for CrossRef data.
 *
 * This function is (very closely) based on islandora_scholar module's
 * islandora_doi_crossref_translator.
 */
function openskydora_crossref_translator ($crossref_data) {
    // Parse the source into a DOMDocument.
    $crossref_xml = new DOMDocument("1.0");
    if (!$crossref_xml->loadXML($crossref_data)) {
        return FALSE;
    }
    // Create MODS XML.
    $mods = new DOMDocument('1.0');
    $mods->loadXML('<mods xmlns="http://www.loc.gov/mods/v3" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:osm="http://nldr.library.ucar.edu/metadata/osm" xmlns:xlink="http://www.w3.org/1999/xlink"/>');

    // @todo Implement book support.

    // Add metadata for journal articles.
    foreach ($crossref_xml->getElementsByTagName('journal') as $journal) {
        $genre = $mods->createElement('genre');
        $genre->nodeValue = 'article';
        $mods->firstChild->appendChild($genre);

        $relateditem = $mods->createElement('relatedItem');
        $mods->firstChild->appendChild($relateditem);
        $relateditem_attribute = $mods->createAttribute('type');
        $relateditem_attribute->value = 'host';
        $relateditem->appendChild($relateditem_attribute);

        foreach ($journal->getElementsByTagName('journal_metadata') as $metadata) {
            foreach ($metadata->childNodes as $child) {
                switch ($child->nodeName) {
                    case 'full_title':
                        $titleinfo = $mods->createElement('titleInfo');
                        $relateditem->appendChild($titleinfo);
                        $title = $mods->createElement('title');
                        $titleinfo->appendChild($title);
                        if (!is_null($child->firstChild)) {
                            $title_text = $mods->importNode($child->firstChild);
                            $title->appendChild($title_text);
                        }
                        break;

                    case 'abbrev_title':
                        $titleinfo = $mods->createElement('titleInfo');
                        $titleinfo_attribute = $mods->createAttribute('type');
                        $titleinfo_attribute->value = 'abbreviated';
                        $titleinfo->appendChild($titleinfo_attribute);
                        $relateditem->appendChild($titleinfo);
                        $title = $mods->createElement('title');
                        $titleinfo->appendChild($title);
                        if (!is_null($child->firstChild)) {
                            $title_text = $mods->importNode($child->firstChild);
                            $title->appendChild($title_text);
                        }
                        break;

                    case 'issn':
                        $identifier = $mods->createElement('identifier');
                        $identifier_attribute = $mods->createAttribute('type');
                        $identifier_attribute->value = 'issn';
                        $identifier->appendChild($identifier_attribute);
                        $relateditem->appendChild($identifier);
                        if (!is_null($child->firstChild)) {
                            $identifier_text = $mods->importNode($child->firstChild);
                            $identifier->appendChild($identifier_text);
                        }
                        break;
                }
            }
        }
        foreach ($journal->getElementsByTagName('journal_issue') as $issue) {
            if ($issue->hasChildNodes()) {
                foreach ($issue->childNodes as $child) {
                    switch ($child->nodeName) {
                        case 'publication_date':
                            if ($child->hasChildNodes()) {
                                foreach ($child->childNodes as $date_part) {
                                    switch ($date_part->nodeName) {
                                        case 'month':
                                            $pubdate_month = $date_part->nodeValue;
                                            break;

                                        case 'year':
                                            $pubdate_year = $date_part->nodeValue;
                                            break;

                                        case 'day':
                                            $pubdate_day = $date_part->nodeValue;
                                            break;
                                    }
                                }
                            }
                            break;

                        case 'journal_volume':
                            if ($child->hasChildNodes()) {
                                foreach ($child->childNodes as $volume) {
                                    switch ($volume->nodeName) {
                                        case 'volume':
                                            if (!is_null($volume->firstChild)) {
                                                $volume_number = $mods->importNode($volume->firstChild);
                                            }
                                            break;
                                    }
                                }
                            }
                            break;

                        case 'issue':
                            if (!is_null($child->firstChild)) {
                                $issue_number = $mods->importNode($child->firstChild);
                            }
                            break;
                    }
                }
            }
        }
        foreach ($journal->getElementsByTagName('journal_article') as $article) {
            if ($article->hasAttributes()) {
                foreach ($article->attributes as $att) {
                    switch ($att->name) {
                        case 'language':
                            $language = $mods->createElement('language');
                            $mods->firstChild->appendChild($language);
                            $languageterm = $mods->createElement('languageTerm');
                            $language->appendChild($languageterm);
                            $language_text = $mods->createTextNode($att->value);
                            $languageterm->appendChild($language_text);
                            if (strlen($att->value) == 2) {
                                $type_attribute = $mods->createAttribute('type');
                                $type_attribute->value = 'code';
                                $languageterm->appendChild($type_attribute);
                                $authority_attribute = $mods->createAttribute('authority');
                                $authority_attribute->value = 'rfc4646';
                                $languageterm->appendChild($authority_attribute);
                            }
                            break;
                    }
                }
            }
            if ($article->hasChildNodes()) {
                foreach ($article->childNodes as $child) {
                    switch ($child->nodeName) {
                        case 'titles':
                            $titleinfo = $mods->createElement('titleInfo');
                            $mods->firstChild->appendChild($titleinfo);
                            foreach ($child->childNodes as $grandchild) {
                                switch ($grandchild->nodeName) {
                                    case 'title':
                                        if (!is_null($grandchild->firstChild)) {
                                            $article_title_text = islandora_doi_filter_html($grandchild);
                                            $article_title = $mods->createElement('title', $article_title_text);
                                            $titleinfo->appendChild($article_title);
                                        }
                                        break;

                                    case 'subtitle':
                                        //  NCAR Library doesn't want subtitles for articles.
                                        /*

                                    if (!is_null($grandchild->firstChild)) {
                                        $article_subtitle_text = islandora_doi_filter_html($grandchild);
                                        $article_subtitle = $mods->createElement('subTitle', $article_subtitle_text);
                                        $titleinfo->appendChild($article_subtitle);
                                      }
                                        */
                                        break;

                                    case 'original_language_title':
                                        $translated_titleinfo = $mods->createElement('titleInfo');
                                        $mods->firstChild->appendChild($translated_titleinfo);
                                        $titleinfo_attribute = $mods->createAttribute('type');
                                        $titleinfo_attribute->value = 'translated';
                                        $translated_titleinfo->appendChild($titleinfo_attribute);
                                        foreach ($grandchild->attributes as $att) {
                                            switch ($att->nodeName) {
                                                case 'language':
                                                    $titleinfo_attribute = $mods->createAttribute('lang');
                                                    $titleinfo_attribute->value = $att->nodeValue;
                                                    $translated_titleinfo->appendChild($titleinfo_attribute);
                                                    break;
                                            }
                                        }
                                        if (!is_null($grandchild->firstChild)) {
                                            $translated_title_text = islandora_doi_filter_html($grandchild);
                                            $translated_title = $mods->createElement('title', $translated_title_text);
                                            $translated_titleinfo->appendChild($translated_title);
                                        }
                                        break;
                                }
                            }
                            break;

                        case 'contributors':
                            foreach ($child->getElementsByTagName('organization') as $organization) {
                                $name = $mods->createElement('name');
                                $mods->firstChild->appendChild($name);
                                $name_attribute = $mods->createAttribute('type');
                                $name_attribute->value = 'corporate';
                                $name->appendChild($name_attribute);
                                $namepart = $mods->createElement('namePart');
                                $name->appendChild($namepart);
                                if (!is_null($organization->firstChild)) {
                                    $namepart_text = $mods->importNode($organization->firstChild);
                                    $namepart->appendChild($namepart_text);
                                }
                            }

                            foreach ($child->getElementsByTagName('person_name') as $person) {
                                $name = $mods->createElement('name');
                                $mods->firstChild->appendChild($name);
                                $name_attribute = $mods->createAttribute('type');
                                $name_attribute->value = 'personal';
                                $name->appendChild($name_attribute);

                                foreach ($person->childNodes as $person_part) {
                                    switch ($person_part->nodeName) {
                                        case 'given_name':
                                            $namepart = $mods->createElement('namePart');
                                            $name->appendChild($namepart);
                                            $namepart_attribute = $mods->createAttribute('type');
                                            $namepart_attribute->value = 'given';
                                            $namepart->appendChild($namepart_attribute);
                                            if (!is_null($person_part->firstChild)) {
                                                $given_name_part = $mods->importNode($person_part->firstChild);
                                                $namepart->appendChild($given_name_part);
                                            }
                                            break;

                                        case 'surname':
                                            $namepart = $mods->createElement('namePart');
                                            $name->appendChild($namepart);
                                            $namepart_attribute = $mods->createAttribute('type');
                                            $namepart_attribute->value = 'family';
                                            $namepart->appendChild($namepart_attribute);
                                            if (!is_null($person_part->firstChild)) {
                                                $surname_part = $mods->importNode($person_part->firstChild);
                                                $namepart->appendChild($surname_part);
                                            }
                                            break;

                                        case 'suffix':
                                            $terms_of_address = $mods->createElement('namePart');
                                            $name->appendChild($terms_of_address);
                                            if (!is_null($person_part->firstChild)) {
                                                $terms_of_address_text = $mods->importNode($person_part->firstChild);
                                                $terms_of_address->appendChild($terms_of_address_text);
                                            }
                                            $terms_of_address_attribute = $mods->createAttribute('type');
                                            $terms_of_address_attribute->value = 'termsOfAddress';
                                            $terms_of_address->appendChild($terms_of_address_attribute);
                                            break;

                                        case 'affiliation':
                                            $affiliation = $mods->createElement('affiliation');
                                            $name->appendChild($affiliation);
                                            if (!is_null($person_part->firstChild)) {
                                                $affiliation_text = $mods->importNode($person_part->firstChild);
                                                $affiliation->appendChild($affiliation_text);
                                            }
                                            break;
                                        case 'ORCID':
                                            $orcid = $mods->createElement('nameIdentifier');
                                            $name->appendChild($orcid);
                                            if (!is_null($person_part->firstChild)) {
                                                $orcid_text = $mods->importNode($person_part->firstChild);
                                                $orcid_text->nodeValue = str_replace('http://orcid.org/', '', (string)$orcid_text->nodeValue);
                                                $orcid->appendChild($orcid_text);
                                            }
                                            $orcid_attribute = $mods->createAttribute('type');
                                            $orcid_attribute->value = 'ORCID';
                                            $orcid->appendChild($orcid_attribute);
                                            break;
                                    }
                                }

                                foreach ($person->attributes as $att) {
                                    switch ($att->nodeName) {
                                        case 'contributor_role':
                                            $role = $mods->createElement('role');
                                            $name->appendChild($role);
                                            $roleterm = $mods->createElement('roleTerm');
                                            $role->appendChild($roleterm);
                                            $roleterm_text = $mods->createTextNode(strtolower($att->nodeValue));
                                            $roleterm->appendChild($roleterm_text);
                                            $roles = array('author', 'editor', 'translator');
                                            if (in_array(strtolower($att->nodeValue), $roles)) {
                                                $roleterm_attribute = $mods->createAttribute('authority');
                                                $roleterm_attribute->value = 'marcrelator';
                                                $roleterm->appendChild($roleterm_attribute);
                                                $roleterm_attribute = $mods->createAttribute('type');
                                                $roleterm_attribute->value = 'text';
                                                $roleterm->appendChild($roleterm_attribute);
                                            }
                                            break;

                                        case 'sequence':
                                            if (strcmp('first', strtolower($att->nodeValue)) == 0) {
                                                $name_attribute = $mods->createAttribute('usage');
                                                $name_attribute->value = 'primary';
                                                $name->appendChild($name_attribute);
                                            }
                                    }
                                }
                            }
                            break;

                        case 'publication_date':
                            // Note that this overwrites the publication date that came
                            // from the journal metadata above.
                            if ($child->hasChildNodes()) {
                                foreach ($child->childNodes as $date_part) {
                                    switch ($date_part->nodeName) {
                                        case 'month':
                                            $pubdate_month = $date_part->nodeValue;
                                            break;

                                        case 'year':
                                            $pubdate_year = $date_part->nodeValue;
                                            break;

                                        case 'day':
                                            $pubdate_day = $date_part->nodeValue;
                                            break;
                                    }
                                }
                            }
                            break;

                        case 'pages':
                            if ($child->hasChildNodes()) {
                                foreach ($child->childNodes as $page_part) {
                                    switch ($page_part->nodeName) {
                                        case 'first_page':
                                            if (!is_null($page_part->firstChild)) {
                                                $first_page = $mods->importNode($page_part->firstChild);
                                                $first_page->nodeValue = str_replace(',', '', $first_page->nodeValue);
                                            }
                                            break;

                                        case 'last_page':
                                            if (!is_null($page_part->firstChild)) {
                                                $last_page = $mods->importNode($page_part->firstChild);
                                                $last_page->nodeValue = str_replace(',', '', $last_page->nodeValue);
                                            }
                                            break;

                                        case 'other_pages':
                                            if (!is_null($page_part->firstChild)) {
                                                $other_pages = $mods->importNode($page_part->firstChild);
                                                $other_pages->nodeValue = str_replace(',', '', $other_pages->nodeValue);
                                            }
                                            break;
                                    }
                                }
                            }
                            break;

                        case 'publisher_item':
                            // @todo
                            if ($child->hasChildNodes()) {
                                foreach ($child->childNodes as $id) {
                                    switch ($id->nodeName) {
                                        case 'item_number':
                                            $article_number = $mods->importNode($id->firstChild);
                                            break;
                                    }
                                }
                            }
                            break;

                        case 'crossmark':
                            // @todo
                            break;

                        case 'doi_data':
                            if ($child->hasChildNodes()) {
                                foreach ($child->childNodes as $identifier) {
                                    switch ($identifier->nodeName) {
                                        case 'doi':
                                            if (!is_null($identifier->firstChild)) {
                                                $doi = $mods->createElement('identifier');
                                                $mods->firstChild->appendChild($doi);
                                                $doi_attribute = $mods->createAttribute('type');
                                                $doi_attribute->value = 'doi';
                                                $doi->appendChild($doi_attribute);
                                                $doi_text = $mods->importNode($identifier->firstChild);
                                                $doi->appendChild($doi_text);

                                                // assign $doi so we can use it
                                                // in post processor
                                                $doi = $doi_text->nodeValue;
                                                dpm ('doi: ' . $doi);
                                            }
                                            break;

                                        case 'resource':
                                            /*
                                        if (!is_null($identifier->firstChild)) {
                                          $uri = $mods->createElement('identifier');
                                          $mods->firstChild->appendChild($uri);
                                          $uri_attribute = $mods->createAttribute('type');
                                          $uri_attribute->value = 'uri';
                                          $uri->appendChild($uri_attribute);
                                          $uri_text = $mods->importNode($identifier->firstChild);
                                          $uri->appendChild($uri_text);
                                        }
                                            */
                                            break;
                                    }
                                }
                            }
                            break;

                        case 'citation_list':
                            // @todo
                            break;

                        case 'component_list':
                            // @todo
                            break;
                        case 'program';
                            // @todo
                            break;
                    }
                }
            }
        }
        // Add the publication date if it was read in one of the two places above.
        $origininfo = $mods->createElement('originInfo');
        $mods->firstChild->appendChild($origininfo);

        if (!empty($pubdate_year)) {
            $date_value = $pubdate_year;
            if (!empty($pubdate_month)) {
                if (strlen($pubdate_month) == 1) {
                    $pubdate_month = "0" . $pubdate_month;
                }
                $date_value .= "-" . $pubdate_month;
            }
            if (!empty($pubdate_day)) {
                if (strlen($pubdate_day) == 1) {
                    $pubdate_day = "0" . $pubdate_day;
                }
                $date_value .= "-" . $pubdate_day;
            }
            $dateissued = $mods->createElement('dateIssued');
            $dateissued_attribute = $mods->createAttribute('encoding');
            $dateissued_attribute->value = 'w3cdtf';
            $dateissued->appendChild($dateissued_attribute);
            $dateissued_attribute = $mods->createAttribute('keyDate');
            $dateissued_attribute->value = 'yes';
            $dateissued->appendChild($dateissued_attribute);
            $origininfo->appendChild($dateissued);
            $dateissued->nodeValue = $date_value;
        }

        // Add the page, volume, and issue data if it was read above.
        if (!empty($volume_number) || !empty($issue_number) || !empty($other_pages) || !empty($last_page) || !empty($first_page) || !empty($pubdate_year) || !empty($pubdate_month) || !empty($pubdate_day) || !empty($article_number)) {
            $part = $mods->createElement('part');
            $relateditem->appendChild($part);

            if (!empty($last_page) || !empty($first_page) || !empty($other_pages)) {
                $extent = $mods->createElement('extent');
                $part->appendChild($extent);
                $extent_attribute = $mods->createAttribute('unit');
                $extent_attribute->value = 'page';
                $extent->appendChild($extent_attribute);

//        if (!empty($other_pages)) {
//          $list = $mods->createElement('list');
//          $list->appendChild($other_pages);
//          $extent->appendChild($list);
//        }
                if (!empty($last_page) && !empty($first_page)) {
                    $testvalue = $first_page->nodeValue . '-' . $last_page->nodeValue;
                    $list = $mods->createElement('list');
                    $list->nodeValue = $first_page->nodeValue . '-' . $last_page->nodeValue;
                    $extent->appendChild($list);
                }
                if (!empty($first_page)) {
                    $start = $mods->createElement('start');
                    $start->appendChild($first_page);
                    $extent->appendChild($start);
                }
                if (!empty($last_page)) {
                    $end = $mods->createElement('end');
                    $end->appendChild($last_page);
                    $extent->appendChild($end);
                }
            }

            if (!empty($volume_number)) {
                $volume = $mods->createElement('detail');
                $part->appendChild($volume);
                $volume_attribute = $mods->createAttribute('type');
                $volume_attribute->value = 'volume';
                $volume->appendChild($volume_attribute);
                $number = $mods->createElement('number');
                $volume->appendChild($number);
                $number->appendChild($volume_number);
            }
            if (!empty($issue_number)) {
                $issue = $mods->createElement('detail');
                $part->appendChild($issue);
                $issue_attribute = $mods->createAttribute('type');
                $issue_attribute->value = 'issue';
                $issue->appendChild($issue_attribute);
                $number = $mods->createElement('number');
                $issue->appendChild($number);
                $number->appendChild($issue_number);
            }
            if (!empty($article_number)) {
                $article_num = $mods->createElement('detail');
                $part->appendChild($article_num);
                $article_num_attribute = $mods->createAttribute('type');
                $article_num_attribute->value = 'citation/article';
                $article_num->appendChild($article_num_attribute);
                $number = $mods->createElement('number');
                $article_num->appendChild($number);
                $number->appendChild($article_number);
            }

            //  NCAR Library doesn't want date in related items.
            //      if (!empty($date_value)) {
            //        $date = $mods->createElement('date');
            //        $date->nodeValue = $date_value;
            //        $part->appendChild($date);
            //      }
        }

        /* Post processor - and maybe some day using a hook_alter structure.
           for now, we will cheat a littlb by using global variables to cache
           info from the crossRef parse.
        */
        openskydora_translator_post_processor ($doi, $mods, $crossref_xml);

        // Return after first instance.
        return $mods;
    }

    return FALSE;
}

/**
 * @param $mods - a DOMDocument
 * @param $crossref - a DOMDocument
 */
function openskydora_translator_post_processor ($doi, $mods, $crossref) {
    dpm ('openskydora_translator_post_processor');

    /*

     WTF??? I could never extract the DOI from the $mods DOMDocument. 
     No idea why. Finally i just passed as param. (but i didn't want
     to because I wanted to set this up as an alter ...)
    
    dpm ('mods is a '. get_class($mods));
    // get the doi from the crossref dom
    $xpath = new DOMXpath($mods);
    $xpath->registerNamespace("ns", $mods->documentElement->namespaceURI);
    $nodes = $xpath->query("//ns:identifier[@type='doi']");

    dpm ('FOUND '. $nodes->length . ' nodes');

    if (empty($nodes) || $nodes->length == 0) {
        dpm( 'DOI not found in MODS');
        error_log( 'DOI not found in MODS');
        return;
    }

    $mods_doi_node = $nodes->item(0);
    if ($mods_doi_node) {
        dpm ('mods_doi_node');
        dpm ($mods_doi_node->saveXML());
        if ($doi) {
            $doi = $mods_doi_node->nodeValue;
            error_dpm ("DOI: $doi\n");
        } else {
            dpm ("DOI NOT FOUND");
            return;
        }
    }
    else {
        dpm ("never found node - BAILING");
        return;
    }
    */

    $wos_dom = openskydora_get_wos_dom ($doi);
    dpm ('wos_dom');
    dpm ($wos_dom);

    $wos_data = openskydora_get_wos_data ($wos_dom);

    // insert ABSTRACT into MODS
    add_wos_abstract_to_mods ($mods, $wos_data['abstract']);

    // get crossref award ids
    $crossref_award_ids = array();

    // get award_ids from crossref
    $xpath = new DOMXpath($crossref);
    $selector = "//program/assertion/assertion[@name='award_number']";
    $nodes = $xpath->query($selector);
    
    if (is_null($nodes)) {
        error_log('award_id not found in CROSSREF');
        return;
    } else {
        foreach ($nodes as $node) {
            $crossref_award_ids[] = $node->nodeValue;
        }
    }

    $merged_award_ids = merge_award_ids($crossref_award_ids, $wos_data['award_ids']);

    $validated_award_ids = array();
    foreach ($merged_award_ids as $award_id) {
        $resp = get_kuali_award_info ($award_id);
        if ($resp) $validated_award_ids[] = $award_id;
    }

    foreach($validated_award_ids as $award_id) {
        // insert into MODS (but what is the desired structure??)
        error_log ("inserting $award_id...\n");
        $award_node = $mods->createElement('note');
        $award_node->setAttribute('type','funding');
        $award_node->nodeValue = $award_id;
        $mods->firstChild->appendChild($award_node);
    }

    //    dpm ($mods->saveXML());

}

// ---- Award_id Utilities --------------------
/**
 * Merge, tokenize, normalize, filter, dedup and sort the combined award_ids
 *
 */
function merge_award_ids ($wos_award_ids, $crossref_award_ids) {
    $merged_award_ids = array_unique(array_merge($crossref_award_ids, $wos_award_ids));
    // $merged_award_ids[] = 'FA9550-16-1-0050';  // one that we KNOW will
    // validate just to test

    $tokenized_award_ids = tokenize_award_ids($merged_award_ids);
    $deduped_award_ids = dedup_award_ids($tokenized_award_ids);

    asort($deduped_award_ids);
    return $deduped_award_ids;
}

/**
 * The vast majority of award_id values obtained from WOS and CrossRef are
 * single grant numbers, but
 * sometimes award_id fields have extra stuff in them in addition to (or maybe
 * instead of)
 * an actual award_id.
 *
 * Our approach is to
 * 1 - "tokenize" the award_id value by splitting it on spaces, then
 * 2 - keep only the tokens that might be award_ids (see get_award_id)

 *
 * @param $value - an award_id field obtained from WOS or CrossRef
 * @return array
 */
function get_award_id_tokens ($value) {
    $values = array_filter(array_map('get_award_id', explode(" ", $value)));
    //    print_r($values);
    return $values;
}

/**
 * Normalize the given $value by removing "special chars" and then, if the
 * result could be an award_id, return it. Otherwise return null.
 *
 * Possible award_ids must have at least 1 numeric char and must be longer than
 * 5 characters (the minumum that Kuali will match).
 *
 * @param $value -
 * @return mixed|null
 */
function get_award_id ($value) {

    $val = preg_replace("/[^A-Za-z0-9 ]/", '', $value);

    if (strlen($val) < 5) {
        return null;
    }

    // if it doesn't have a number, reject it
    preg_match("/[0-9]/", $val, $matches);

    return (sizeof($matches) < 1) ? null : $val;
}

/**
 * Given a list of award_fields obtained from WOS or CrossRef, tokenize the
 * award_fields and
 * merge them all into a list of possible award_ids.
 *
 * @param $award_fields
 * @return array
 */
function tokenize_award_ids($award_fields) {

    $tokens = array();
    forEach ($award_fields as $raw_award_field) {
        $tokens = array_merge ($tokens, get_award_id_tokens($raw_award_field));
    }
    return $tokens;
}

/**
 * Remove items that are right-most substrings of another item in the given
 * array
 *
 * @param $award_ids
 */
function dedup_award_ids ($award_ids) {
    $award_ids = array_unique($award_ids);
    $deduped = array();
    forEach ($award_ids as $candidate) {
        $is_right_subset = false;
        forEach ($award_ids as $id) {
            if ((strlen($id) > strlen($candidate)) && endswith($id, $candidate)) {
                $is_right_subset = true;
            }
        }
        if (!$is_right_subset) {
            $deduped[] = $candidate;
        }
    }
    return $deduped;
}
