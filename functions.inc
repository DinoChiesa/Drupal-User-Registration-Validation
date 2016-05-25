<?php
/**
 * @file functions.inc
 * supporting functions for Dpchiesa user registration validation
 * 
 * used in the .module and the .install
 *
 */


function dpchiesa_validate_userreg_available_checks() {
  // a check returns true when the regex matches the subject. 
  $checks = Array();
  $checks[] = Array('varname' => 'name_disallow_spaces',
                    'description' => 'disallow spaces in usernames',
                    'reason' => 'spaces are not allowed in the username',
                    'subject' => 'name',
                    'regex' => '/\s+/');
  $checks[] = Array('varname' => 'name_word_chars_only',
                    'description' => 'disallow non-word characters in usernames',
                    'reason' => 'spaces and most non-word characters are disallowed in the username',
                    'subject' => 'name', 
                    'negate' => true,
                    'regex' => '/^\pL[\pL0-9_]+$/');
  $checks[] = Array('varname' => 'firstname_word_chars_only',
                    'description' => 'disallow non-word characters in usernames',
                    'reason' => 'non-word characters are not allowed in the firstname',
                    'subject' => 'field_first_name',
                    'negate' => true,
                    'regex' => '/^\pL[\pL ]+$/');
  $checks[] = Array('varname' => 'lastname_word_chars_only',
                    'description' => 'disallow non-word characters in usernames',
                    'reason' => 'non-word characters are not allowed in the lastname',
                    'subject' => 'field_last_name',
                    'negate' => true,
                    'regex' => '/^\pL[\pL ]+$/');
  $checks[] = Array('varname' => 'email_disallow_free',
                    'description' => 'disallow free email (yahoo, gmail, hotmail, mailinator, etc)',
                    'reason' => 'invalid email',
                    'subject' => 'mail', 
                    'regex' => '/@(hotmail|yahoo|googlemail|gmail|mailinator)\./');
  return $checks;
}



function dpchiesa_validate_userreg_form_value($form_values, $name) {
  // In some cases $form_values[$check['subject']] is returning an array.
  // This is because of Drupals auto-translation. 
  // Coerce to a string to allow preg_match to work without warnings.
  if (!isset($form_values[$name])) {
    return NULL;
  }
  $value = $form_values[$name];
  if (is_array($value)) {
    if (isset($value['und'][0]['value'])) {
      $value = $value['und'][0]['value'];
    }
    else {
      $value = var_export($value, true);
    }
  } 
  return $value;
}

function dpchiesa_validate_userreg_apply_check($form_values, $check) {
  // a check returns true when the regex matches the subject.
  // TRUE implies invalid form data.
  // This function will need to get smarter if new checks not involving regex are added.

  $subject = dpchiesa_validate_userreg_form_value($form_values, $check['subject']);
  if ($subject == NULL) { return FALSE; }
  
  $result = preg_match($check['regex'], $subject);
  if (isset($check['negate']) && $check['negate']) {
    $result = !$result;
  }
  return $result;
}


function dpchiesa_validate_userreg_evaluate_check($form_values, $check) {
  // TRUE means invalid
  $prefix = 'dpchiesa_validate_userreg';
  $varname = $prefix . '_enabled_' . $check['varname'];
  $check_enabled = variable_get($varname, 0);  
  $result = $check_enabled && dpchiesa_validate_userreg_apply_check($form_values, $check);
  // watchdog('userreg_validation',
  //          "check !check, enabled: !enabled, value: !value, result: !result",
  //          array('!check' => $check['varname'],
  //                '!enabled' => $check_enabled,
  //                '!value' => dpchiesa_validate_userreg_form_value($form_values, $check['subject']),
  //                '!result' => var_export($result,true)),
  //          WATCHDOG_WARNING);
  return $result;
}


function dpchiesa_validate_userreg_evaluate_all_enabled_checks($form_values) {
  $available_checks = dpchiesa_validate_userreg_available_checks();
  foreach ($available_checks as $check) {
    if (dpchiesa_validate_userreg_evaluate_check($form_values, $check)) { 
      form_set_error($check['subject'], t($check['reason']));
    }
  }
}


?>
