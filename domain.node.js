/**
 * @file
 * Attaches behaviours for the Domain Access module
 */

(function ($) {

  /**
   * Provide the summary information for the domain access vertical tab
   * on the node edit form.
   */
  Backdrop.behaviors.domainNodeFieldsetSummaries = {
    attach: function (context) {
      $('fieldset.domain-access-options-form').backdropSetSummary(function (context) {

        if (typeof Backdrop.settings.domain !== 'undefined') {
          var fieldTypeSelector = (Backdrop.settings.domain.fieldType == 0) ? ':checked' : ':selected';
          var allAffiliatesCheckbox = $('.form-item-domain-site input', context);
          var domainInput = (fieldTypeSelector == ':checked') ? $('.form-item-domains input', context) : $('.form-item-domains select option', context);
          domainInput = domainInput.filter(fieldTypeSelector);
          var selectedDomainCount = domainInput.length;

          if (allAffiliatesCheckbox.is(':checked')) {
            return Backdrop.t('All affiliates');
          }
          else if (selectedDomainCount == 0) {
            return Backdrop.t('No affiliates');
          }
          else if (selectedDomainCount == 1) {
            return (fieldTypeSelector == ':checked') ? domainInput.siblings('label').first().text() : domainInput.first().text();
          }
          else {
            return Backdrop.t('@count affiliates', {'@count': selectedDomainCount});
          }
        }

      });
    }
  };

})(jQuery);
