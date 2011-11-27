<?php
/**
 * Darts competition module: Admin functions.
 */

/**
 * Create the form to add or edit a result between two players.
 *
 * @param array $form_state The state of the form.
 * @param array $result The result to edit, or null if a new result is being created.
 * @return array The result edit form.
 */
function darts_competition_result_edit($form_state, $result = array()) {
    $form = array();

    // Store the result
    $form['result'] = array(
        '#type' => 'value',
        '#value' => $result
    );
    $form['result_id'] = array(
        '#type' => 'value',
        '#value' => !empty($result) ? $result['result_id'] : 0
    );

    $form['result_date'] = array(
        '#title' => t('Date'),
        '#type' => 'date',
        '#required' => true,
    );
    if (!empty($result)) {
        $form['result_date']['#default_value'] = array(
            'year' => format_date($result['result_date'], 'custom', 'Y'),
            'month' => format_date($result['result_date'], 'custom', 'n'),
            'day' => format_date($result['result_date'], 'custom', 'j')
        );
    }

    // Fieldset with result details for the home player.
    $form['home'] = darts_competition_result_player_fieldset('home_', $result);
    $form['home']['#title'] = t('Player one');

    // Fielset with result details for the away player.
    $form['away'] = darts_competition_result_player_fieldset('away_', $result);
    $form['away']['#title'] = t('Player two');

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save result'),
        '#prefix' => '<div style="clear: both;">',
        '#suffix' => '</div>'
    );

    return $form;
}

/**
 * Create the fieldset that contains the player data for a given result.
 *
 * @param string $prefix Prefix to use for player specific fields.
 * @param array $result The result for which the fieldset is created, or null.
 * @return array The fieldset built for the given prefix.
 *
 * @see darts_match_edit_form()
 */
function darts_competition_result_player_fieldset($prefix, $result = array()) {
    $limit = ceil(variable_get('darts_competition_result_best_of', 5) / 2);

    $set = array(
        '#type' => 'fieldset',
        '#prefix' => '<div style="float: left; width: 50%;">',
        '#suffix' => '</div>'
    );

    $set[$prefix.'uid'] = array(
        '#type' => 'select',
        '#options' => darts_get_players(true),
        '#default_value' => !empty($result) ? $result[$prefix.'uid'] : 0
    );

    $set[$prefix.'legs'] = array(
        '#type' => 'numberfield',
        '#title' => t('Score'),
        '#size' => 5,
        '#maxlength' => 3,
        '#default_value' => !empty($result) ? $result[$prefix.'legs'] : 0
    );

    $set['statistics'] = array(
        '#type' => 'fieldset',
        '#title' => t('Statistics'),
        '#theme' => 'darts_competition_result_statistics_fieldset'
    );

    $set['statistics'][$prefix.'throws_100'] = array(
        '#type' => 'numberfield',
        '#title' => t('100+ throws'),
        '#size' => 5,
        '#maxlength' => 3,
        '#default_value' => !empty($result) ? $result[$prefix.'throws_100'] : 0
    );

    $set['statistics'][$prefix.'throws_140'] = array(
        '#type' => 'numberfield',
        '#title' => t('140+ throws'),
        '#size' => 5,
        '#maxlength' => 3,
        '#default_value' => !empty($result) ? $result[$prefix.'throws_140'] : 0
    );

    $set['statistics'][$prefix.'throws_180'] = array(
        '#type' => 'numberfield',
        '#title' => t('180 throws'),
        '#size' => 5,
        '#maxlength' => 3,
        '#default_value' => !empty($result) ? $result[$prefix.'throws_180'] : 0
    );

    // Prepare list of statistics
    $count = is_array($result[$prefix.'finishes']) ? $limit - count($result[$prefix.'finishes'])  : $limit;
    for ($i = -1; $i >= -$count; $i--) {
        $result[$prefix.'finishes'][$i] = 0;
    }
    $count = is_array($result[$prefix.'darts']) ? $limit - count($result[$prefix.'darts'])  : $limit;
    for ($i = -1; $i >= -$count; $i--) {
        $result[$prefix.'darts'][$i] = 0;
    }

    $set['statistics'][$prefix.'finishes'] = array(
        '#tree' => true
    );
    foreach ($result[$prefix.'finishes'] as $stat_id => $stat_value) {
        $set['statistics'][$prefix.'finishes'][$stat_id] = array(
            '#type' => 'numberfield',
            '#size' => 5,
            '#maxlength' => 3,
            '#default_value' => $stat_value,
        );
    }

    $set['statistics'][$prefix.'darts'] = array(
        '#tree' => true
    );
    foreach ($result[$prefix.'darts'] as $stat_id => $stat_value) {
        $set['statistics'][$prefix.'darts'][$stat_id] = array(
            '#type' => 'numberfield',
            '#size' => 5,
            '#maxlength' => 3,
            '#default_value' => $stat_value,
        );
    }

    return $set;
}

/**
 * Validate user input for the result edit form.
 *
 * This checks if the players are two different, existing players and whether all numerical input is valid.
 *
 * @param $form The form.
 * @param $form_state The submitted form data.
 * @return void Nothing.
 */
function darts_competition_result_edit_validate($form, &$form_state) {
    if ($form_state['values']['home_uid'] == 0) {
        form_set_error('home][home_uid', t('You have not selected a player. Please select a player for the result.'));
    }
    elseif ($form_state['values']['away_uid'] == 0) {
        form_set_error('away][away_uid', t('You have not selected a player. Please select a player for the result.'));
    }
    elseif ($form_state['values']['home_uid'] == $form_state['values']['away_uid']) {
        form_set_error('home][home_uid', t('You have selected the same player for both options. Please correct the issue.'));
    }
}

/**
 * Process the submitted result edit form.
 *
 * Prepare the submitted data to be entered into the database.
 *
 * @param $form The form.
 * @param $form_state The submitted form data.
 * @return void Nothing.
 */
function darts_competition_result_edit_submit($form, &$form_state) {
    $old_result = $form_state['result'];
    // Save the result
    $result = array(
        'result_id' => $form_state['values']['result_id'],
        'result_date' => mktime(20, 0, 0, $form_state['values']['result_date']['month'], $form_state['values']['result_date']['day'], $form_state['values']['result_date']['year']),
        'home_uid' => $form_state['values']['home_uid'],
        'home_legs' => $form_state['values']['home_legs'],
        'away_uid' => $form_state['values']['away_uid'],
        'away_legs' => $form_state['values']['away_legs']
    );
    $key = ($form_state['values']['result_id'] > 0) ? array('result_id') : array();
    $status = drupal_write_record('darts_competition_results', $result, $key);
    if ($status === false) {
        drupal_set_message('Could not save the result.', 'error');
    }

    // Home players statistics
    $result_stats = array(
        'result_id' => $result['result_id'],
        'uid' => $result['home_uid'],
        'throw_100' => $form_state['values']['home_throws_100'],
        'throw_140' => $form_state['values']['home_throws_140'],
        'throw_180' => $form_state['values']['home_throws_180']
    );
    $key = ($form_state['values']['result_id'] > 0) ? array('result_id', 'uid') : array();
    $status = drupal_write_record('darts_competition_result_stats', $result_stats, $key);
    if ($status === false) {
        drupal_set_message('Could not save throws statistics for player one.', 'error');
    }

    $status = true;
    foreach ($form_state['values']['home_finishes'] as $stat_id => $stat_value) {
        if ($stat_value <= 0) continue;
        if ($status === false) {
            drupal_set_message('Could not save finish statistics for player one.', 'error');
            break;
        }

        $unique_stat = array(
            'stat_id' => $stat_id,
            'result_id' => $result['result_id'],
            'uid' => $result['home_uid'],
            'stat_type' => DARTS_STAT_FINISH,
            'stat_value' => $stat_value
        );
        $key = ($stat_id > 0) ? array('stat_id') : array();
        $status = drupal_write_record('darts_competition_result_unique_stats', $unique_stat, $key);
    }

    $status = true;
    foreach ($form_state['values']['home_darts'] as $stat_id => $stat_value) {
        if ($stat_value <= 0) continue;
        if ($status === false) {
            drupal_set_message('Could not save darts statistics for player one.', 'error');
            break;
        }

        $unique_stat = array(
            'stat_id' => $stat_id,
            'result_id' => $result['result_id'],
            'uid' => $result['home_uid'],
            'stat_type' => DARTS_STAT_DARTS,
            'stat_value' => $stat_value
        );
        $key = ($stat_id > 0) ? array('stat_id') : array();
        $status = drupal_write_record('darts_competition_result_unique_stats', $unique_stat, $key);
    }

    // Throws (away player)
    $result_stats = array(
        'result_id' => $result['result_id'],
        'uid' => $result['away_uid'],
        'throw_100' => $form_state['values']['away_throws_100'],
        'throw_140' => $form_state['values']['away_throws_140'],
        'throw_180' => $form_state['values']['away_throws_180']
    );
    $key = ($form_state['values']['result_id'] > 0) ? array('result_id', 'uid') : array();
    $status = drupal_write_record('darts_competition_result_stats', $result_stats, $key);
    if ($status === false) {
        drupal_set_message('Could not save throws statistics for player two.', 'error');
    }

    $status = true;
    foreach ($form_state['values']['away_finishes'] as $stat_id => $stat_value) {
        if ($stat_value <= 0) continue;
        if ($status === false) {
            drupal_set_message('Could not save finish statistics for player two.', 'error');
            break;
        }

        $unique_stat = array(
            'stat_id' => $stat_id,
            'result_id' => $result['result_id'],
            'uid' => $result['away_uid'],
            'stat_type' => DARTS_STAT_FINISH,
            'stat_value' => $stat_value
        );
        $key = ($stat_id > 0) ? array('stat_id') : array();
        $status = drupal_write_record('darts_competition_result_unique_stats', $unique_stat, $key);
    }

    $status = true;
    foreach ($form_state['values']['away_darts'] as $stat_id => $stat_value) {
        if ($stat_value <= 0) continue;
        if ($status === false) {
            drupal_set_message('Could not save darts statistics for player two.', 'error');
            break;
        }

        $unique_stat = array(
            'stat_id' => $stat_id,
            'result_id' => $result['result_id'],
            'uid' => $result['away_uid'],
            'stat_type' => DARTS_STAT_DARTS,
            'stat_value' => $stat_value
        );
        $key = ($stat_id > 0) ? array('stat_id') : array();
        $status = drupal_write_record('darts_competition_result_unique_stats', $unique_stat, $key);
    }

    // Update ranking for home player
    $data = db_query('SELECT * FROM {darts_competition_ranking} WHERE uid = %d', $result['home_uid']);
    $player = db_fetch_array($data);
    // New player, create new ranking record.
    if ($player === false) {
        $player = array(
            'uid' => $result['home_uid'],
            'games_played' => 1,
            'games_won' => ($result['home_legs'] > $result['away_legs']) ? 1 : 0,
            'legs_played' => $result['home_legs'] + $result['away_legs'],
            'legs_won' => $result['home_legs']
        );
        $key = array();
    }
    // Update record from existing result
    elseif (!empty($old_result)) {
        // Remove old data from the record
        $player['games_won'] -= ($old_result['home_legs'] > $old_result['away_legs']) ? 1 : 0;
        $player['legs_played'] -= $old_result['home_legs'] + $old_result['away_legs'];
        $player['legs_won'] -= $old_result['home_legs'];
        // Update record with new data
        $player['games_won'] += ($result['home_legs'] > $result['away_legs']) ? 1 : 0;
        $player['legs_played'] += $result['home_legs'] + $result['away_legs'];
        $player['legs_won'] += $result['home_legs'];
    }
    // Update record
    else {
        $player['games_played'] += 1;
        $player['games_won'] += ($result['home_legs'] > $result['away_legs']) ? 1 : 0;
        $player['legs_played'] += $result['home_legs'] + $result['away_legs'];
        $player['legs_won'] += $result['home_legs'];
        $key = array('uid');
    }
    $status = drupal_write_record('darts_competition_ranking', $player, $key);
    if ($status === false) {
        drupal_set_message('Could not save ranking for player one.', 'error');
    }

    // Update ranking for away player
    $data = db_query('SELECT * FROM {darts_competition_ranking} WHERE uid = %d', $result['away_uid']);
    $player = db_fetch_array($data);
    // New player, create new ranking record.
    if ($player === false) {
        $player = array(
            'uid' => $result['away_uid'],
            'games_played' => 1,
            'games_won' => ($result['home_legs'] < $result['away_legs']) ? 1 : 0,
            'legs_played' => $result['home_legs'] + $result['away_legs'],
            'legs_won' => $result['away_legs']
        );
        $key = array();
    }
    // Update record from existing result
    elseif (!empty($old_result)) {
        // Remove old data from the record
        $player['games_won'] -= ($old_result['home_legs'] < $old_result['away_legs']) ? 1 : 0;
        $player['legs_played'] -= $old_result['home_legs'] + $old_result['away_legs'];
        $player['legs_won'] -= $old_result['away_legs'];
        // Update record with new data
        $player['games_won'] += ($result['home_legs'] < $result['away_legs']) ? 1 : 0;
        $player['legs_played'] += $result['home_legs'] + $result['away_legs'];
        $player['legs_won'] += $result['away_legs'];
    }
    // Update record
    else {
        $player['games_played'] += 1;
        $player['games_won'] += ($result['home_legs'] < $result['away_legs']) ? 1 : 0;
        $player['legs_played'] += $result['home_legs'] + $result['away_legs'];
        $player['legs_won'] += $result['away_legs'];
        $key = array('uid');
    }
    $status = drupal_write_record('darts_competition_ranking', $player, $key);
    if ($status === false) {
        drupal_set_message('Could not save ranking for player two.', 'error');
    }

    // Go to result
    drupal_set_message(t('Result saved.'));
    drupal_goto('darts/competition/results/'.$result['result_id']);
}

/**
 * Theme function for the statistics fieldset.
 *
 * @param array $fieldset The fieldset.
 * @return string Generated HTML for the fieldset.
 */
function theme_darts_competition_result_statistics_fieldset($fieldset) {
    $content = '';
    $prefix = is_array($fieldset['home_finishes']) ? 'home_' : 'away_';

    $content .= drupal_render($fieldset[$prefix.'throws_100']);
    $content .= drupal_render($fieldset[$prefix.'throws_140']);
    $content .= drupal_render($fieldset[$prefix.'throws_180']);

    if (is_array($fieldset[$prefix.'finishes']) && !empty($fieldset[$prefix.'finishes'])) {
        $cells = array();
        foreach (element_children($fieldset[$prefix.'finishes']) as $key) {
            $cells[] = drupal_render($fieldset[$prefix.'finishes'][$key]);
        }
        $table = array(
            '#type' => 'item',
            '#title' => t('Finish scores'),
            '#value' => theme_table(array(), array($cells))
        );
        $content .= drupal_render($table);
    }
    else {
        $content .= t('No finish data available');
    }

    if (is_array($fieldset[$prefix.'darts']) && !empty($fieldset[$prefix.'darts'])) {
        $cells = array();
        foreach (element_children($fieldset[$prefix.'darts']) as $key) {
            $cells[] = drupal_render($fieldset[$prefix.'darts'][$key]);
        }
        $table = array(
            '#type' => 'item',
            '#title' => t('Darts to finish'),
            '#value' => theme_table(array(), array($cells))
        );
        $content .= drupal_render($table);
    }
    else {
        $content .= t('No finish data available');
    }

    $content .= drupal_render($fieldset);

    return $content;
}

/**
 * Build the settings form for the local darts competition.
 *
 * @param array $form_state The form state.
 * @return array Forms API elements, passed through system_settings_form().
 */
function darts_competition_admin_settings($form_state) {
    $form = array();

    $best_of_options = array();
    for ($i = 3; $i < 10; $i += 2) {
        $best_of_options[$i] = t('Best of !index', array('!index' => $i));
    }
    $form['darts_competition_result_best_of'] = array(
        '#type' => 'select',
        '#title' => t('Match rules'),
        '#options' => $best_of_options,
        '#default_value' => variable_get('darts_competition_result_best_of', 5)
    );

    $form['darts_competition_reset'] = array(
        '#type' => 'item',
        '#title' => t('Restart competition'),
        '#value' => l(t('Click to restart the competition'), 'admin/darts/competition/restart')
    );

    return system_settings_form($form);
}

/**
 * Results management form.
 *
 * @param array $form_state The form state.
 * @return array Forms API elements.
 */
function darts_competition_admin_results($form_state) {
    $form = array();
    $results = array();

    $data = pager_query("SELECT r.*, h.name AS home_name, a.name AS away_name
                         FROM {darts_competition_results} r
                         LEFT JOIN {users} h ON r.home_uid = h.uid
                         LEFT JOIN {users} a ON r.away_uid = a.uid
                         ORDER BY r.result_date DESC", 25);

    while (($result = db_fetch_array($data)) !== false) {
        $result_id = $result['result_id'];
        $results[$result_id] = '';
        $form['result_date'][$result_id]['#value'] = l(format_date($result['result_date'], 'custom', 'd-m-Y'), 'darts/competition/result/'.$result_id);
        $form['home_player'][$result_id]['#value'] = theme('username', (object) array('uid' => $result['home_uid'], 'name' => $result['home_name']));
        $form['away_player'][$result_id]['#value'] = theme('username', (object) array('uid' => $result['away_uid'], 'name' => $result['away_name']));
        $form['result_score'][$result_id]['#value'] = sprintf('%d - %d', $result['home_legs'], $result['away_legs']);
        $form['result_actions'][$result_id]['#value'] = l(t('edit'), 'darts/competition/results/'.$result_id.'/edit');
    }

    if (empty($results)) {
        drupal_set_message(t('No results found.'));
    }

    $form['results'] = array(
        '#type' => 'checkboxes',
        '#options' => $results
    );

    $form['actions'] = array(
        '#type' => 'fieldset',
        '#title' => t('With selected results')
    );
    $form['actions']['delete_results'] = array(
        '#type' => 'submit',
        '#value' => t('Delete')
    );

    $form['pager']['#value'] = theme('pager', 25);

    return $form;
}

function darts_competition_admin_results_validate($form, &$form_state) {
    $form_state['values']['results'] = array_filter($form_state['values']['results']);
    if (empty($form_state['values']['results'])) {
        form_set_error('', t('No results selected.'));
    }
}

function darts_competition_admin_results_submit($form, &$form_state) {
    $form_state['values']['results'] = array_filter($form_state['values']['results']);

    if ($form_state['values']['op'] == $form_state['values']['delete_results']) {
        foreach ($form_state['values']['results'] as $result_id) {
            darts_competition_result_delete($result_id);
        }
        drupal_set_message(t('Selected results deleted.'));
    }
}

function theme_darts_competition_admin_results($form) {
    $header = array(
        theme('table_select_header_cell'),
        t('Date'),
        t('Home player'),
        t('Away player'),
        t('Score'),
        t('Actions')
    );
    $rows = array();

    if (is_array($form['result_date']) && !empty($form['result_date'])) {
        foreach (element_children($form['result_date']) as $result_id) {
            $rows[] = array(
                drupal_render($form['results'][$result_id]),
                drupal_render($form['result_date'][$result_id]),
                drupal_render($form['home_player'][$result_id]),
                drupal_render($form['away_player'][$result_id]),
                drupal_render($form['result_score'][$result_id]),
                drupal_render($form['result_actions'][$result_id])
            );
        }
        $output  = theme('table', $header, $rows);
        $output .= drupal_render($form);
    }
    else {
        $output = '';
    }

    return $output;
}

/**
 * Confirmation page to restart the local darts competition.
 *
 * @seealso darts_competition_confirm_restart_submit().
 *
 * @param array $form_state The form state.
 * @return array Forms API elements generated by confirm_form().
 */
function darts_competition_confirm_restart($form_state) {
    return confirm_form(array(), t('Are you sure you want to restart the competition?'), 'admin/darts/competition');
}

/**
 * Restart the local darts competition.
 *
 * This function is the submit handler for darts_competition_confirm_restart().
 *
 * @param array $form The form.
 * @param array $form_state The form state.
 * @return void Nothing.
 */
function darts_competition_confirm_restart_submit($form, &$form_state) {
    db_query('TRUNCATE TABLE {darts_competition_results}');
    db_query('TRUNCATE TABLE {darts_competition_ranking}');
    db_query('TRUNCATE TABLE {darts_competition_result_stats}');
    db_query('TRUNCATE TABLE {darts_competition_result_unique_stats}');

    drupal_set_message(t('Competition successfully restarted.'));
    drupal_goto('admin/darts/competition');
}