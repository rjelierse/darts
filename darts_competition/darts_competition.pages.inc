<?php
/**
 * Darts competition module: Display functions.
 */
 
/**
 * Display a list of competition results.
 *
 * The list is paginated, but cannot be filtered or sorted.
 *
 * @return string The rendered HTML.
 */
function darts_competition_results() {
    $data = pager_query("SELECT r.*, h.name AS home_name, a.name AS away_name
                         FROM {darts_competition_results} r
                         LEFT JOIN {users} h ON r.home_uid = h.uid
                         LEFT JOIN {users} a ON r.away_uid = a.uid
                         ORDER BY r.result_date DESC", 20);

    $header = array(
        t('Date'),
        array('data' => t('Players'), 'colspan' => 2),
        t('Score'),
//        ''
    );
    $rows = array();
    while (($result = db_fetch_array($data)) !== false) {
        // Home player user object
        $home_player = new stdClass();
        $home_player->uid = $result['home_uid'];
        $home_player->name = $result['home_name'];
        // Away player user object
        $away_player = new stdClass();
        $away_player->uid = $result['away_uid'];
        $away_player->name = $result['away_name'];

        $rows[] = array(
            array('data' => format_date($result['result_date'], 'custom', 'j F Y'), 'class' => 'numeric date'),
            array('data' => theme('username', $home_player), 'class' => ($result['home_legs'] > $result['away_legs']) ? 'winner' : ''),
            array('data' => theme('username', $away_player), 'class' => ($result['home_legs'] < $result['away_legs']) ? 'winner' : ''),
            array('data' => sprintf('%d - %d', $result['home_legs'], $result['away_legs']), 'class' => 'numeric'),
//            array('data' => l(t('details'), 'darts/competition/results/' . $result['result_id']))
        );
    }

    if (empty($rows)) {
        drupal_set_message(t('No results found.'));
        return '';
    }

    $content = theme('table', $header, $rows);
    $content .= theme('pager');

    return $content;
}

/**
 * Display the ranking for the competition.
 *
 * Ranking is create based on the the amount of games that have been won by player.
 *
 * @return string Rendered HTML.
 */
function darts_competition_ranking() {
    $data = db_query("SELECT r.*, u.name FROM {darts_competition_ranking} r LEFT JOIN {users} u ON r.uid = u.uid ORDER BY games_won DESC, games_played ASC");

    $header = array(
        t('Rank'),
        t('Name'),
        t('Games played'),
        t('Games won'),
        t('Legs played'),
        t('Legs won')
    );

    $rank = 1;
    $rows = array();
    while (($record = db_fetch_object($data)) !== false) {
        $rows[] = array(
            array('data' => $rank.'.', 'class' => 'numeric'),
            theme_username($record),
            array('data' => $record->games_played, 'class' => 'numeric'),
            array('data' => $record->games_won, 'class' => 'numeric'),
            array('data' => $record->legs_played, 'class' => 'numeric'),
            array('data' => $record->legs_won, 'class' => 'numeric')
        );
        $rank++;
    }

    if (empty($rows)) {
        drupal_set_message(t('No players found.'));
        return '';
    }

    return theme_table($header, $rows);
}



/**
 * View the competition statistics.
 *
 * @return string The rendered HTML.
 */
function darts_competition_statistics($type) {
    $items = array();
    if ($type == 'throws') {
        $items['#prefix'] = '<div class="three-column-layout">';
        $items['#suffix'] = '</div>';
        $items['throw_100'] = array('#title' => t('100+ throws'), '#weight' => -50);
        $items['throw_140'] = array('#title' => t('140+ throws'), '#weight' => -49);
        $items['throw_180'] = array('#title' => t('180 throws'), '#weight' => -48);

        $header = array(
            t('Rank'),
            t('Player'),
            t('Count')
        );

        foreach (array('throw_100', 'throw_140', 'throw_180') as $field) {
            $items[$field]['#type'] = 'section';
            $items[$field]['#attributes'] = array('class' => 'column');

            $data = db_query('SELECT s.uid, u.name, SUM(%s) AS throws FROM {darts_competition_result_stats} s LEFT JOIN {users} u ON s.uid = u.uid GROUP BY s.uid ORDER BY throws DESC', $field);
            if ($data === false) {
                $items[$field]['#value'] = t('No statistics found');
                continue;
            }

            $i = 1;
            $rows = array();

            while(($record = db_fetch_array($data)) !== false) {
                $rows[] = array(
                    array('data' => $i++.'.', 'class' => 'numeric'),
                    theme('username', (object) $record),
                    array('data' => $record['throws'], 'class' => 'numeric'),
                );
            }

            if (empty($rows)) {
                $items[$field]['#value'] = t('No statistics found');
                continue;
            }

            $items[$field]['#value'] = theme('table', $header, $rows);
        }
    }
    else {
        $items['#prefix'] = '<div class="two-column-layout">';
        $items['#suffix'] = '</div>';
        $items[DARTS_STAT_FINISH] = array('#title' => t('Highest finishes'), '#weight' => -50);
        $items[DARTS_STAT_DARTS] = array('#title' => t('Fastest finishes'), '#weight' => -49);

        $header = array(
            t('Rank'),
            t('Player')
        );

        foreach(array(DARTS_STAT_FINISH, DARTS_STAT_DARTS) as $stat_type) {
            $items[$stat_type]['#type'] = 'section';
            $items[$stat_type]['#attributes'] = array('class' => 'column');
            $header[2] = ($stat_type == DARTS_STAT_FINISH) ? t('Score') : t('Darts');

            $data = db_query('SELECT s.uid, u.name, s.stat_value FROM {darts_competition_result_unique_stats} s LEFT JOIN {users} u ON s.uid = u.uid WHERE s.stat_type = %d ORDER BY s.stat_value DESC LIMIT 25', $stat_type);
            if ($data === false) {
                $items[$stat_type]['#value'] = t('No statistics found');
                continue;
            }

            $i = 1;
            $rows = array();

            while(($record = db_fetch_array($data)) !== false) {
                $rows[] = array(
                    array('data' => $i++.'.', 'class' => 'numeric'),
                    theme('username', (object) $record),
                    array('data' => $record['stat_value'], 'class' => 'numeric'),
                );
            }

            if (empty($rows)) {
                $items[$stat_type]['#value'] = t('No statistics found');
                continue;
            }

            $items[$stat_type]['#value'] = theme('table', $header, $rows);
        }
    }
    
    return drupal_render($items);
}