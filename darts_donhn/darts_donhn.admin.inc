<?php
/**
 * Darts DONHN competition module: Administration functions.
 */

/**
 * Build the DONHN competition settings form and pass it through system_settings_form().
 *
 * @param $form_state The form state.
 * @return array The form.
 */
function darts_donhn_admin_settings($form_state) {
    $form = array();

    $form['reset_donhn'] = array(
        '#type' => 'item',
        '#title' => t('Reset the DONHN competition'),
        '#value' => l(t('Click to reset the DONHN competition'), 'admin/darts/donhn/reset')
    );

    return system_settings_form($form);
}

function darts_donhn_competition_confirm_reset($form_state) {
    $form = array();
    $form['extra'] = array(
        '#value' => t('This will remove all') . theme('item_list', array(t('Teams'), t('Results'), t('Statistics')))
    );
    return confirm_form($form, t('Are you sure you want to reset the DONHN competition?'), 'admin/darts/donhn');
}

function darts_donhn_competition_confirm_reset_submit ($form, &$form_state) {
    db_query('TRUNCATE TABLE {darts_donhn_teams}');
    db_query('TRUNCATE TABLE {darts_donhn_team_players}');
    db_query('TRUNCATE TABLE {darts_donhn_results}');
    db_query('TRUNCATE TABLE {darts_donhn_result_stats}');

    drupal_set_message(t('DONHN competition reset.'));
    drupal_goto('admin/darts/donhn');
}

/**
 * Build the administration page for managing the teams.
 *
 * @param $form_state The form state.
 * @return array The form.
 */
function darts_donhn_admin_teams($form_state) {
    $form = array();

    $data = db_query('SELECT t.team_id, t.team_name, t.division, t.captain_uid, u.name captain_name FROM {darts_donhn_teams} t LEFT JOIN {users} u ON t.captain_uid = u.uid ORDER BY t.team_name ASC');
    if ($data === false) {
        return false;
    }

    $teams = array();
    while (($team = db_fetch_array($data)) !== false) {
        $teams[$team['team_id']] = '';
        $form['team_name'][$team['team_id']]['#value'] = l($team['team_name'], 'darts/donhn/' . $team['team_id']);
        $form['division'][$team['team_id']]['#value'] = check_plain($team['division']);
        $form['captain'][$team['team_id']]['#value'] = theme('username', (object) array('uid' => $team['captain_uid'], 'name' => $team['captain_name']));
        $actions = array(
            l(t('edit'), 'darts/donhn/'. $team['team_id'] .'/edit'),
            l(t('add result'), 'darts/donhn/'. $team['team_id'] .'/add-result')
        );
        $form['team_actions'][$team['team_id']]['#value'] = implode(', ', $actions);
    }

    if (empty($teams)) {
        drupal_set_message(t('No teams found.'));
    }

    $form['teams'] = array(
        '#type' => 'checkboxes',
        '#options' => $teams
    );

    $form['actions'] = array(
        '#type' => 'fieldset',
        '#title' => t('With selected teams')
    );

    return $form;
}

/**
 * Theme the team administration form.
 *
 * @param $form The team administration form.
 * @return string The rendered HTML.
 */
function theme_darts_donhn_admin_teams($form) {
    $header = array(
        theme('table_select_header_cell'),
        t('Team'),
        t('Division'),
        t('Captain'),
        t('Actions')
    );
    $rows = array();
    if (is_array($form['teams']) && !empty($form['teams'])) {
        foreach (element_children($form['teams']) as $team_id) {
            $rows[] = array(
                drupal_render($form['teams'][$team_id]),
                drupal_render($form['team_name'][$team_id]),
                drupal_render($form['division'][$team_id]),
                drupal_render($form['captain'][$team_id]),
                drupal_render($form['team_actions'][$team_id]),
            );
        }
    }

    $content = theme('table', $header, $rows);
    $content .= drupal_render($form);
    return $content;
}

/**
 * Build the form for editing teams in the DONHN competition.
 *
 * @param array $form_state The form state.
 * @param array $team (optional) The team to edit.
 * @return array The form.
 */
function darts_donhn_team_edit($form_state, $team = array()) {
    $form = array();

    ahah_helper_register($form, $form_state);

    if (isset($form_state['storage']['division'])) {
        $division = $form_state['storage']['division'];
    }
    elseif (!empty($team)) {
        $division = $team['division'];
    }
    else {
        $division = 0;
    }

    if (isset($form_state['storage']['team_name'])) {
        $team_name = $form_state['storage']['team_name'];
    }
    elseif (!empty($team)) {
        $team_name = $team['team_name'];
    }
    else {
        $team_name = 0;
    }

    $form['#prefix'] = '<div id="darts-team-wrapper">';
    $form['#suffix'] = '</div>';
    $form['#tree'] = true;

    $form['team_id'] = array(
        '#type' => 'hidden',
        '#value' => !empty($team) ? $team['team_id'] : 0
    );

    $form['division'] = array(
        '#type' => 'select',
        '#title' => t('Division'),
        '#options' => darts_donhn_competition_get_divisions(),
        '#default_value' => $division,
        '#required' => true,
        '#ahah' => array(
            'event' => 'change',
            'path' => ahah_helper_path(),
            'wrapper' => 'darts-team-wrapper'
        )
    );
    // Update button if Javascript is not available
    $form['division_update'] = array(
        '#type' => 'submit',
        '#value' => t('Update division'),
        '#submit' => array('ahah_helper_generic_submit'),
        '#attributes' => array('class' => 'no-js')
    );
    // Only show this part if the division has been set.
    if (!empty($division)) {
        $form['team_name'] = array(
            '#type' => 'select',
            '#title' => t('Team name'),
            '#options' => darts_donhn_competition_get_teams($division),
            '#default_value' => $team_name,
            '#required' => true
        );
        // Add AHAH routine if the team is being created
        if (empty($team)) {
            $form['team_name']['#ahah'] = array(
                'event' => 'change',
                'path' => ahah_helper_path(),
                'wrapper' => 'darts-team-wrapper'
            );
            // Update button if Javascript is not available
            $form['team_name_update'] = array(
                '#type' => 'submit',
                '#value' => t('Update team name'),
                '#submit' => array('ahah_helper_generic_submit'),
                '#attributes' => array('class' => 'no-js')
            );
        }
    }
    // Only show this part if the team name has been set.
    if (!empty($team_name)) {
        $form['captain_uid'] = array(
            '#type' => 'select',
            '#title' => t('Team captain'),
            '#options' => darts_get_players(true),
            '#default_value' => isset($form_state['storage']['captain_uid']) ? $form_state['storage']['captain_uid'] : (!empty($team['captain_uid']) ? $team['captain_uid'] : 0),
            '#required' => true
        );

        $form['contact'] = array(
            '#type' => 'fieldset',
            '#title' => t('Contact information'),
            '#description' => t('Information on how this team can be contacted. Most likely the contact information of the team captain.'),
            '#tree' => false
        );
        $form['contact']['captain_mail'] = array(
            '#type' => 'emailfield',
            '#title' => t('E-Mail address'),
            '#default_value' => isset($form_state['storage']['location']['captain_mail']) ? $form_state['storage']['location']['captain_mail'] : $team['captain_mail'],
            '#required' => true
        );
        $form['contact']['captain_phone'] = array(
            '#type' => 'telfield',
            '#title' => t('Telephone number'),
            '#maxlength' => 10,
            '#default_value' => isset($form_state['storage']['location']['captain_phone']) ? $form_state['storage']['location']['captain_phone'] : $team['captain_phone'],
            '#required' => true
        );

        $form['location'] = array(
            '#type' => 'fieldset',
            '#title' => t('Location'),
            '#description' => t('Information on the location where the team plays its home matches.'),
            '#tree' => false
        );
        $form['location']['location_name'] = array(
            '#type' => 'textfield',
            '#title' => t('Name'),
            '#default_value' => isset($form_state['storage']['location']['location_name']) ? $form_state['storage']['location']['location_name'] : $team['location_name'],
            '#required' => true
        );
        $form['location']['location_address'] = array(
            '#type' => 'textfield',
            '#title' => t('Address'),
            '#default_value' => isset($form_state['storage']['location']['location_address']) ? $form_state['storage']['location']['location_address'] : $team['location_address'],
            '#required' => true
        );
        $form['location']['location_city'] = array(
            '#type' => 'textfield',
            '#title' => t('City'),
            '#default_value' => isset($form_state['storage']['location']['location_city']) ? $form_state['storage']['location']['location_city'] : $team['location_city'],
            '#required' => true
        );
        $form['location']['location_phone'] = array(
            '#type' => 'telfield',
            '#title' => t('Telephone number'),
            '#maxlength' => 10,
            '#default_value' => isset($form_state['storage']['location']['location_phone']) ? $form_state['storage']['location']['location_phone'] : $team['location_phone']
        );
    }

    $form['save'] = array(
        '#type' => 'submit',
        '#value' => t('Save team'),
        '#disabled' => (empty($division) || empty($team_name))
    );

    $form['#submit'][] = 'darts_donhn_team_edit_submit';

    return $form;
}

/**
 * Process the submitted data from the team edit form.
 *
 * @param $form The form.
 * @param $form_state The form state.
 * @return void Nothing.
 */
function darts_donhn_team_edit_submit($form, &$form_state) {
    $fields = array('team_id', 'team_name', 'division', 'captain_uid', 'captain_mail', 'captain_phone', 'location_name', 'location_address', 'location_city', 'location_phone');

    $team = array();
    foreach ($fields as $field) {
        $team[$field] = $form_state['values'][$field];
    }

    $key = ($team['team_id'] > 0) ? array('team_id') : array();

    if (drupal_write_record('darts_donhn_teams', $team, $key) === false) {
        drupal_set_message(t('Could not save team.'), 'error');
        drupal_goto('admin/content/darts/teams');
    }

    $path = 'darts/donhn/'.$team['team_id'];

    // Build menu item
    $link = array(
        'link_path' => $path,
        'link_title' => $team['team_name'],
        'menu_name' => 'primary-links'
    );

    // Search for existing menu item
    $mlid = db_result(db_query("SELECT mlid FROM {menu_links} WHERE link_path = '%s'", $path));
    if ($mlid !== false) {
        $link['mlid'] = $mlid;
    }

    // Save the menu item
    menu_link_save($link);

    drupal_set_message(t('Team was saved successfully.'));
    drupal_goto($path);
}
/**
 * Build the form to edit a team result for the DONHN competition.
 *
 * @todo Add statistics.
 *
 * @param array $form_state The form state.
 * @param array $team The team to edit.
 * @param array $result (optional) The results to edit.
 * @return array The form.
 */
function darts_donhn_result_edit($form_state, $team, $result = array()) {
    $form = array();

    $form['team_id'] = array(
        '#type' => 'value',
        '#value' => $team['team_id']
    );
    $form['result_id'] = array(
        '#type' => 'value',
        '#value' => !empty($result) ? $result['result_id'] : 0
    );

    $form['result_date'] = array(
        '#type' => 'date',
        '#title' => t('Date'),
        '#required' => true
    );
    if (!empty($result)) {
        $form['result_date']['#default_value'] = array(
            'year' => format_date($result['result_date'], 'custom', 'Y'),
            'month' => format_date($result['result_date'], 'custom', 'n'),
            'day' => format_date($result['result_date'], 'custom', 'j')
        );
    }

    $form['result'] = array(
        '#type' => 'fieldset',
        '#title' => t('Result')
    );
    $form['result']['team_name'] = array('#value' => check_plain($team['team_name']));
    $form['result']['team_score'] = array(
        '#type' => 'numberfield',
        '#size' => 5,
        '#maxlength' => 1,
        '#default_value' => !empty($result) ? $result['team_score'] : 0,
        '#required' => true
    );
    $form['result']['opponent_name'] = array(
        '#type' => 'select',
        '#options' => darts_donhn_competition_get_teams($team['division']),
        '#default_value' => !empty($result) ? $result['opponent_name'] : 0,
        '#required' => true
    );
    $form['result']['opponent_score'] = array(
        '#type' => 'numberfield',
        '#size' => 5,
        '#maxlength' => 1,
        '#default_value' => !empty($result) ? $result['opponent_score'] : 0,
        '#required' => true
    );
    $form['result']['result_type'] = array(
        '#type' => 'radios',
        '#title' => t('Location'),
        '#options' => array(
            DARTS_DONHN_RESULT_HOME => t('Home'),
            DARTS_DONHN_RESULT_AWAY => t('Away')
        ),
        '#default_value' => !empty($result) ? $result['result_type'] : DARTS_DONHN_RESULT_HOME,
        '#required' => true
    );

    $form['save'] = array(
        '#type' => 'submit',
        '#value' => t('Save result')
    );

    return $form;
}

function darts_donhn_result_edit_validate($form, &$form_state) {
    if ($form_state['values']['team_name'] == $form_state['values']['opponent_name']) {
        form_set_error('result][opponent_name', t('The opposing team you selected is the same as the team for which you are trying to enter the result.'));
    }
    elseif (empty($form_state['values']['opponent_name'])) {
        form_set_error('result][opponent_name', t('You have not selected an opposing team.'));
    }
    elseif (($form_state['values']['team_score'] + $form_state['values']['opponent_score']) <> 9) {
        form_set_error('result][team_score', t('The combined scores do not equal 9.'));
    }
}

function darts_donhn_result_edit_submit($form, &$form_state) {
    $fields = array('result_id', 'team_id', 'team_score', 'opponent_name', 'opponent_score', 'result_type');

    // Fill a result record to write to the database.
    $result = array();
    foreach ($fields as $field) {
        $result[$field] = $form_state['values'][$field];
    }
    $result['result_date'] = mktime(20, 0, 0, $form_state['values']['result_date']['month'], $form_state['values']['result_date']['day'], $form_state['values']['result_date']['year']);
    $key = ($result['result_id'] > 0) ? array('result_id') : array();

    if (drupal_write_record('darts_donhn_results', $result, $key) === false) {
        form_set_error('', t('Could not save the result.'));
    }
    else {
        drupal_set_message('Result saved.');
        drupal_goto('darts/donhn/' . $result['team_id'] . '/results/' . $result['result_id']);
    }
}