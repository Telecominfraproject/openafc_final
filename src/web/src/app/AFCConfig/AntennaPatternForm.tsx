import * as React from 'react';
import { FormGroup, FormSelect, FormSelectOption, TextInput, Tooltip, TooltipPosition } from '@patternfly/react-core';
import { OutlinedQuestionCircleIcon } from '@patternfly/react-icons';
import { AntennaPatternState } from '../Lib/RatApiTypes';

/**
 * AntennaPatternForm.tsx: sub form of afc config form
 * author: Sam Smucny
 */

/**
 * Sub form component for Antenna patterns
 */
export default class AntennaPatternForm extends React.PureComponent<{
  data: AntennaPatternState;
  region: string;
  onChange: (x: AntennaPatternState) => void;
}> {
  private setKind = (s: string) => {
    switch (s) {
      case 'F.1245':
        this.props.onChange({ defaultAntennaPattern: 'F.1245', userUpload: this.props.data.userUpload });
        break;
      case 'F.699':
        this.props.onChange({ defaultAntennaPattern: 'F.699', userUpload: this.props.data.userUpload });
        break;
      case 'WINNF-AIP-07':
        this.props.onChange({ defaultAntennaPattern: 'WINNF-AIP-07', userUpload: this.props.data.userUpload });
        break;
      case 'WINNF-AIP-07-CAN':
        this.props.onChange({ defaultAntennaPattern: 'WINNF-AIP-07-CAN', userUpload: this.props.data.userUpload });
        break;

      default:
      // Do nothing
    }
  };

  setUserUpload = (s: string) =>
    this.props.onChange({
      defaultAntennaPattern: this.props.data.defaultAntennaPattern,
      userUpload: s != 'None' ? { kind: 'User Upload', value: s } : { kind: s, value: '' },
    });

  /**
   * Enumeration of antenna pattern types
   */
  private getPatternOptionsByRegion = (region: string) => {
    if (region.endsWith('CA')) {
      return ['WINNF-AIP-07-CAN', 'F.699', 'F.1245'];
    } else if (region.endsWith('BR')) {
      return ['WINNF-AIP-07', 'F.699', 'F.1245'];
    } else {
      return ['WINNF-AIP-07', 'F.699', 'F.1245'];
    }
  };

  render = () => (
    <FormGroup label="Default FS Receiver Antenna Pattern" fieldId="horizontal-form-antenna">
      {' '}
      <FormSelect
        value={this.props.data.defaultAntennaPattern}
        onChange={(x) => this.setKind(x)}
        id="horzontal-form-antenna"
        name="horizontal-form-antenna"
        isValid={this.props.data.defaultAntennaPattern !== undefined}
        style={{ textAlign: 'right' }}
      >
        {this.getPatternOptionsByRegion(this.props.region).map((option) => (
          <FormSelectOption key={option} value={option} label={option} />
        ))}
      </FormSelect>
    </FormGroup>
  );
}
