<?php
/**
 * Darts DONHN competition module: Display functions.
 */

/**
 * Team landing page. Shows the results and the current ranking.
 *
 * @param array $team
 *     The team to display the results and ranking for.
 *
 * @return string
 *     The rendered HTML.
 */
function darts_donhn_team_results($team) {
    $items = array();
    $ranking = darts_donhn_competition_get_ranking($team['division']);

    $header = array(
        t('Date'),
        t('Home'),
        t('Away'),
        t('Score'),
//        '',
    );
    $rows = array();
    $results = db_query('SELECT * FROM {darts_donhn_results} WHERE team_id = %d ORDER BY result_date DESC', $team['team_id']);
    if ($results === false) {
        drupal_set_message(t('Could not find results.'), 'warning');
    }
    else {
        while (($result = db_fetch_array($results)) !== false) {
            $home_team_classes = $away_team_classes = array();
            // Prepare variables
            if ($result['result_type'] == DARTS_DONHN_RESULT_HOME) {
                $home_team = $team['team_name'];
                $home_team_classes[] = 'active';
                $away_team = $result['opponent_name'];
                if ($result['team_score'] > $result['opponent_score']) {
                    $home_team_classes[] = 'winner';
                }
                else {
                    $away_team_classes[] = 'winner';
                }
                $score = $result['team_score'] .' - ' . $result['opponent_score'];
            }
            else {
                $away_team = $team['team_name'];
                $away_team_classes[] = 'active';
                $home_team = $result['opponent_name'];
                if ($result['team_score'] > $result['opponent_score']) {
                    $away_team_classes[] = 'winner';
                }
                else {
                    $home_team_classes[] = 'winner';
                }
                $score = $result['opponent_score'] .' - ' . $result['team_score'];
            }
            // Build table row
            $rows[] = array(
                array('data' => format_date($result['result_date'], 'custom' , 'j F Y'), 'class' => 'numeric date'),
                array('data' => $home_team, 'class' => implode(' ', $home_team_classes)),
                array('data' => $away_team, 'class' => implode(' ', $away_team_classes)),
                array('data' => $score, 'class' => 'numeric'),
//                array('data' => l(t('details'), 'darts/donhn/'. $team['team_id'] .'/results/'. $result['result_id']))
            );
        }
        $items['results'] = array(
            '#type' => 'section',
            '#title' => t('Results'),
            '#value' => empty($rows) ? t('Could not find resuls for this team.') : theme('table', $header, $rows),
            '#weight' => -50
        );
    }

    if ($ranking === false) {
        drupal_set_message(t('Could not get ranking.'), 'warning');
    }
    else {
        $rank = 1;
        $header = array(
            array('data' => t('Rank')),
            array('data' => t('Team')),
            array('data' => t('Games played')),
            array('data' => t('Points')),
            array('data' => t('Penalties'))
        );
        $rows = array();
        foreach ($ranking as $ranked) {
            $row = array(
                array('data' =>$rank.'.', 'class' => 'numeric'),
                $ranked['name'],
                array('data' =>$ranked['played'], 'class' => 'numeric'),
                array('data' =>$ranked['points'], 'class' => 'numeric'),
                array('data' =>$ranked['penalty'], 'class' => 'numeric')
            );
            if ($ranked['name'] == $team['team_name']) {
                $rows[] = array(
                    'data' => $row,
                    'class' => 'active'
                );
            }
            else {
                $rows[] = $row;
            }

            $rank++;
        }
        $items['ranking'] = array(
            '#type' => 'section',
            '#title' => t('Ranking'),
            '#value' => theme('table', $header, $rows),
            '#weight' => -49
        );
    }

    return drupal_render($items);
}

/**
 * Team information page.
 *
 * @param array $team
 *     The team to display the information for.
 *
 * @return string
 *     The rendered HTML.
 */
function darts_donhn_team_information($team) {
    $captain = user_load($team['captain_uid']);

    $items = array();
    $items['competition'] = array(
        '#type' => 'user_profile_category',
        '#title' => t('Competition'),
        '#weight' => -50
    );
    $items['competition']['association'] = array(
        '#type' => 'user_profile_item',
        '#title' => t('Association'),
        '#value' => l('DONHN', 'http://www.donhn.nl/', array('attributes' => array('rel' => 'external', 'target' => '_blank'))),
        '#weight' => -50
    );
    $items['competition']['division'] = array(
        '#type' => 'user_profile_item',
        '#title' => t('Division'),
        '#value' => check_plain($team['division']),
        '#weight' => -49
    );

    $items['contact'] = array(
        '#type' => 'user_profile_category',
        '#title' => t('Contact'),
        '#weight' => -40
    );
    $items['contact']['captain'] = array(
        '#type' => 'user_profile_item',
        '#title' => t('Captain'),
        '#value' => theme('username', $captain),
        '#weight' => -50
    );
    $items['contact']['mail'] = array(
        '#type' => 'user_profile_item',
        '#title' => t('E-Mail address'),
        '#value' => str_replace('@', '&#64;', l($team['captain_mail'], 'mailto:' . $team['captain_mail'])),
        '#weight' => -49
    );
    $items['contact']['phone'] = array(
        '#type' => 'user_profile_item',
        '#title' => t('Telephone number'),
        '#value' => check_plain($team['captain_phone']),
        '#weight' => -48
    );

    $items['location'] = array(
        '#type' => 'user_profile_category',
        '#title' => t('Location'),
        '#value' => theme('team_location_information', $team),
        '#weight' => -30
    );
    $items['location']['name'] = array(
        '#type' => 'user_profile_item',
        '#title' => t('Name'),
        '#value' => $team['location_name'],
        '#weight' => -50
    );
    $items['location']['address'] = array(
        '#type' => 'user_profile_item',
        '#title' => t('Address'),
        '#value' => check_plain($team['location_address']) . '<br />' . check_plain($team['location_city']).'<br />'.l(t('find on map'), 'http://maps.google.nl', array('query' => array('q' => $team['location_address'].', '.$team['location_city'], 'z' => 17), 'attributes' => array('rel' => 'external', 'target' => '_blank'))),
        '#weight' => -49
    );
    if (!empty($team['location_phone'])) {
        $items['location']['phone'] = array(
            '#type' => 'user_profile_item',
            '#title' => t('Telephone number'),
            '#value' => check_plain($team['location_phone']),
            '#weight' => -48
        );
    }
    $items['players'] = array(
        '#type' => 'section',
        '#title' => t('Players'),
        '#value' => theme('user_list', $team['players']),
        '#weight' => -45
    );

    return drupal_render($items);
}

/**
 * Show statistics for the given team.
 *
 * @param array $team
 *     The team that is being viewed.
 *
 * @return string
 *     The rendered HTML.
 *
 * @todo Actually make this work.
 */
function darts_donhn_team_statistics ($team) {
    drupal_set_message(t('No statistics found.'));
    return '';
}

/**
 * Show links to the defined DONHN teams.
 *
 * @return string
 *     The rendered HTML.
 */
function darts_donhn_teams () {
    $data = db_query('SELECT team_id, team_name, division FROM {darts_donhn_teams} ORDER BY team_name ASC');
    if ($data === false) {
        drupal_set_message(t('Could not look up teams.'), 'error');
        return '';
    }

    $teams = array();
    while (($team = db_fetch_array($data)) !== false) {
        $teams[] = array(
            'title' => $team['team_name'],
            'href' => 'darts/donhn/' . $team['team_id'],
            'description' => t('Division !division', array('!division' => $team['division'])),
            'localized_options' => array()
        );
    }

    return theme('admin_block_content', $teams);
}