// toggle-chk.js
// ------------------------------------------------------------------
//
// functions to toggle the checked state of checkboxes.
//
// The Drupal FAPI is too cumbersome to use for setting up
// mutually exclusive checkboxes.
//
// created: Wed Aug  3 15:53:33 2016
// last saved: <2016-August-03 16:17:04>

(function ($){
  'use strict';
  var attrName = 'data-unset-if-checked';
  $(function() {
    // for all checkbox that have this attr
    $('div input['+attrName+'][type="checkbox"]').change(function () {
      if (this.checked) {
        var nameOfOtherElement = $(this).attr(attrName);
        $('div input[name="'+ nameOfOtherElement +'"][type="checkbox"]').prop("checked", false);
      }
    });
  });
}(jQuery));
