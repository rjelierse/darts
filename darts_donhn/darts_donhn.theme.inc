<?php
/**
 * Darts DONHN competition module: Theme functions.
 */

/**
 * Prepare the result information for display in the template.
 *
 * @param array $variables
 *     The variables that are passed to the template.
 * 
 * @return void
 *     Nothing.
 */
function template_preprocess_darts_donhn_result (&$variables) {
    $variables['result_date'] = format_date($variables['result']['result_date'], 'custom', 'l j F Y');
    
    if ($variables['result']['result_type'] == DARTS_DONHN_RESULT_HOME) {
        // Home team is team
        $variables['home_team'] = $variables['team']['team_name'];
        $variables['home_team_score'] = $variables['result']['team_score'];
        $variables['home_team_classes'] = ($variables['result']['team_score'] > $variables['result']['opponent_score']) ? 'winner' : '';
        // Away team is opponent
        $variables['away_team'] = $variables['result']['opponent_name'];
        $variables['away_team_score'] = $variables['result']['opponent_score'];
        $variables['away_team_classes'] = ($variables['result']['team_score'] > $variables['result']['opponent_score']) ? '' : 'winner';
    }
    else {
        // Away team is team
        $variables['away_team'] = $variables['team']['team_name'];
        $variables['away_team_score'] = $variables['result']['team_score'];
        $variables['away_team_classes'] = ($variables['result']['team_score'] > $variables['result']['opponent_score']) ? 'winner' : '';
        // Home team is opponent
        $variables['home_team'] = $variables['result']['opponent_name'];
        $variables['home_team_score'] = $variables['result']['opponent_score'];
        $variables['home_team_classes'] = ($variables['result']['team_score'] > $variables['result']['opponent_score']) ? '' : 'winner';
    }
}