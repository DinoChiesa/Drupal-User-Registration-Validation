<?php
/**
 * @file functions.inc
 * supporting functions for Dpchiesa user registration validation
 * 
 * used in the .module and the .install
 *
 */


function dpchiesa_validate_userreg_available_checks() {
  // A check must return true when the data is invalid.
  // 
  // Today, checks are based on regex, and follow this pattern:
  //   varname : the name (suffix) of the drupal variable that stores the enabled status of the check
  //   description : human-readable description of what the check does, presented to the admin.
  //   reason : human-readable description of the check violation, presented to the user when the check trips
  //   regex : a regular expression
  //   subject : the property in the $form_state['values'] array that the regex is compared against
  // 
  // follow the pattern in which when the regex matches the subject.
  $checks = array();
  $checks[] = array('varname' => 'name_disallow_spaces',
                    'description' => 'disallow spaces in usernames',
                    'reason' => 'spaces are not allowed in the username',
                    'subject' => 'name',
                    'regex' => '/\s+/');
  $checks[] = array('varname' => 'name_word_chars_only',
                    'description' => 'disallow non-word characters and spaces in usernames',
                    'reason' => 'spaces and most non-word characters are disallowed in the username',
                    'subject' => 'name', 
                    'negate' => true,
                    'regex' => '/^\pL[\pL0-9_]+$/');
  $checks[] = array('varname' => 'firstname_word_chars_only',
                    'description' => 'disallow non-word characters in usernames',
                    'reason' => 'non-word characters are not allowed in the firstname',
                    'subject' => 'field_first_name',
                    'negate' => true,
                    'regex' => '/^\pL[\pL ]+$/'); // \pL means "word character" incl Unicode
  $checks[] = array('varname' => 'lastname_word_chars_only',
                    'description' => 'disallow non-word characters in usernames',
                    'reason' => 'non-word characters are not allowed in the lastname',
                    'subject' => 'field_last_name',
                    'negate' => true,
                    'regex' => '/^\pL[\pL ]+$/');

  $domains = array('yahoo',
           'googlemail', 'gmail',
           'hotmail', 'live', 'outlook',
           'dispostable', 'maildrop', 'tafmail', '6paq', 'mailinator', 'harakirimail', 'yopmail', '33mail');
  $slug = implode('|', $domains);
  $checks[] = array('varname' => 'email_disallow_free',
                    'description' => 'disallow free email (yahoo, gmail, hotmail, mailinator, etc)',
                    'reason' => 'invalid email',
                    'subject' => 'mail', 
                    'regex' => '/@(' . $slug . ')\./');
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
    $value = (isset($value['und'][0]['value'])) ? $value['und'][0]['value'] : var_export($value, true);
  } 
  return $value;
}


function dpchiesa_validate_userreg_apply_check($form_values, $check) {
  // A check returns true when the regex matches the subject.
  // TRUE implies invalid form data.
  //
  // Today, all checks are regex based. 
  // This function will need to get smarter if other kinds of checks are added.

  $subject = dpchiesa_validate_userreg_form_value($form_values, $check['subject']);
  if ($subject === NULL) {
    return FALSE;
  }
  
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
